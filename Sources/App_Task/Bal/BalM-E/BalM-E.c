// BalM.c
#include "BalM-E.h"
#include "SchIf.h"
#include "Ecu.h"
#include "BatterySt.h"
#include "BmsSysSt.h"
#include "BatterySysSt.h"
#include "ChargeSt.h"
#include "ErrorAlarm.h"
#include "EE.h"
#include "IntCan.h"
#include "WdgIf.h"
#include "AppRelay.h"
#include "LvM.h"

#include "Hvm.h"


#ifdef USE_BALM_E


#if(ECU_RAM_USE_MODE)      
    #ifdef ECU_RAM_USE_GPGAE
    #pragma DATA_SEG __GPAGE_SEG PAGED_RAM 
    #endif
    #ifdef ECU_RAM_USE_RPGAE
    #pragma DATA_SEG __RPAGE_SEG PAGED_RAM 
    #endif      
#else
    #pragma DATA_SEG DEFAULT 
#endif

typedef struct
{
    uByte cnt;
    uByte ReceiveCmdCnt;
    
} sBalM_TmCntType;

sBalM_TmCntType BalM_TmCnt;

typedef struct
{
    uByte  StopBalRslt[MAX_PACK_NUM];
    uByte  CalcBalRslt[MAX_PACK_NUM];
    uByte  StartBalRslt[MAX_PACK_NUM];
    uByte  ForceBalRslt[MAX_PACK_NUM];

    uByte  StopBalRsltCnt;
    uByte  CalcBalRsltCnt;
    uByte  StartBalRsltCnt;
    uByte  ForceBalRsltCnt;
} sBalM_RxSlaType;

sBalM_RxSlaType BalM_RxSla;

typedef struct
{
    uByte BatSt;
    uByte BalSt;
    uByte CalcBal;
    uByte StartBal;
    uByte BalCtrl;
    uByte TxBalCmd;
    uByte RxBalCmd;
    uByte UpdateFlag;
    uByte BatStandByFlag;

    uWord AvgV;
    uWord MinV;
    uWord MaxV;
    uLWord BatCap;
    uWord BalMode;
    uByte BalCmd;
    uByte ForceBal;
    
    uWord ForceBalNo;
    uWord ForceBalTime;
    uLWord SaveFlag1;
    uLWord SaveFlag2;
    uLWord SaveFlag3;
} sBalM_CtrlType;

sBalM_CtrlType BalM_Ctrl;

typedef struct
{
    uWord SysStartCnt;
    
    uWord TimeCnt10s;
    uWord TimeCnt5Min;

    uWord TotalDchBalCnt[MAX_PACK_NUM][MAX_PACK_CELL_NUM];
    uWord TotalChgBalCnt[MAX_PACK_NUM][MAX_PACK_CELL_NUM];

    uWord AfterChgCnt;
    uWord VoltHighCnt;
    uWord StartBalCnt;
    uWord CalcBalCnt;

    uWord UpdateBalStCnt;
    uWord VoltBalCnt;
    uWord StopBalCnt;
} sBalM_BalCntType;

sBalM_BalCntType BalM_BalCnt;                                                     

#pragma DATA_SEG DEFAULT

// balance control current
#define CHG_CUR_2A     OFFSET_CUR - 20
#define DCH_CUR_10A    OFFSET_CUR + 100

// balance control time, 200ms per count
#define BALM_TIME_2S      10
#define BALM_TIME_5S      25
#define BALM_TIME_10S     50
#define BALM_TIME_20S     100
#define BALM_TIME_1MIN    300
#define BALM_TIME_2MIN    600
#define BALM_TIME_3MIN    900
#define BALM_TIME_5MIN    1500
#define BALM_TIME_10MIN   3000

// balance time count, 10s per count
#define BALTIMECNT_5MIN    30
#define BALTIMECNT_90MIN   540
#define BALTIMECNT_150MIN  900
#define BALTIMECNT_210MIN  1260
#define BALTIMECNT_1HOUR   360
#define BALTIMECNT_2HOUR   720
#define BALTIMECNT_3HOUR   1080
#define BALTIMECNT_5HOUR   1800
#define BALTIMECNT_7HOUR   2520
#define BALTIMECNT_10HOUR  3600
#define BALTIMECNT_12HOUR  4320
#define BALTIMECNT_90HOUR  32400
#define BALTIMECNT_120HOUR 43200

// battery static time
#define BALM_ONE_HOUR     3600U   // unit:1s
#define BALM_TWO_HOUR     7200U
#define BALM_240_HOUR     432000U // 24 * 5d=120H

// force balance mask
#define FORCE_BAL_MASK  0x0fff

// balance control flag
#define MASK_CAL_BAL 		  0x01
#define MASK_START_BAL	  0x02
#define MASK_STOP_BAL		  0x04
#define MASK_FORCE_BAL    0x08
#define MASK_CLEAR_BAL    0x10
#define MASK_FORCE_ONE_BAL 0x20

#define ALL_BAL_CLEAR  0xFE
#define ALL_BAL_OPEN   0xFA
#define ONE_BAL_OPEN   0xFC

#define BALM_FLAG_DISABLE 0
#define BALM_FLAG_ENABLE  1
#define BALM_FLAG_FAIL 0
#define BALM_FLAG_OK 1


// balance judge result
#define NEED_BAL      0x01
#define ALL_BAT_LOW   0x02
#define ALL_BAT_HIGH  0x04
#define DONT_NEED_BAL 0x08

// balance on off status
#define ST_BAL_OFF    0x00
#define ST_BAL_ON     0x01

// chg and dch balance status
#define DCH_BAL_OFF   0x00
#define DCH_BAL_ON    0x20
#define DCH_BAL_ERR   0x08
#define CHG_BAL_OFF   0x00
#define CHG_BAL_ON    0x10
#define CHG_BAL_ERR   0x04

/* define balance control commad for LTC6803 */
#define CMD_DCH_BAL_OFF   0x10
#define CMD_DCH_BAL_ON    0x11

#define CMD_CHG_BAL_OFF   0x20
#define CMD_CHG_BAL_ON    0x21

// battery status
#define BALM_BAT_NOT_CHG      0x00
#define BALM_BAT_CHG_NOW      0x01
#define BALM_BAT_CHG_END      0x02
#define BALM_BAT_AFTER_CHG    0x03
#define BALM_BAT_DCH_NOW      0x04
#define BALM_BAT_STANDBY      0x05

//值为1、2、3、4，代表开机电压计算均衡，开机容量计算均衡，充满电压计算均衡，充满容量计算均衡  
//有模式3以上，就不能触发模式1、2；有模式2以上，就不能触发模式1,均衡时间全部完成的时候，计算均衡模式清0
//calc_bal_mode
#define CALC_BAL_MODE_A 1
#define CALC_BAL_MODE_B 2
#define CALC_BAL_MODE_C 3
#define CALC_BAL_MODE_D 4

static void BalM_ProcBalTime(void);

static void BalM_SendBalCmd(void);
static void BalM_SendCmd_CalcBal(void);
static void BalM_SendCmd_StartBal(void);
static void BalM_SendCmd_StopBal(void);
static void BalM_SendCmd_ForceBal(void);

static void BalM_ReceiveBalCmd(void);
static void BalM_ProcBal(void);
static uByte BalM_CalcBal(void);
static void BalM_StartBal(void);
static void BalM_StopBal(void);
static void BalM_ForceBal(void);
static void BalM_ClearBal(void);
static uWord BalM_GetCellSoc(uWord volt);
static uWord BalM_GetCellCap(uWord volt, uLWord bat_actual_ah);
static uWord BalM_SaveBalTime(void);

void BalM_ModuleInit(void)
{
    uWord i;
    GeneralReturnType rslt;
    uByte * RAMPTR p_data;
    uByte bmu_no;
    uByte dlen;
    uByte data[4] ={0,0,0,0 };
    uWord bat_num;
    //uWord wtmp;

    BalM_TmCnt.cnt = 0;
    BalM_TmCnt.ReceiveCmdCnt = 0;

    p_data = (uByte* RAMPTR)(&BalM_RxSla);
    for(i = 0; i < sizeof(BalM_RxSla); i++)
    {
        *(p_data++) = 0;
    }

    p_data = (uByte* RAMPTR)(&BalM_Ctrl);
    for(i = 0; i < sizeof(BalM_Ctrl); i++)
    {
        *(p_data++) = 0;
    }

    p_data = (uByte* RAMPTR)(&BalM_BalCnt);
    for(i = 0; i < sizeof(BalM_BalCnt); i++)
    {
        *(p_data++) = 0;
    }

    /* end of init remian balance time */   

    /* init balance flag */
    bmu_no = 0;

    bat_num = data[1] + (data[0] << 8);

    /* end of init balance flag */

    /* init total balance time */
    rslt = EE_LoadVar(SYSTEMST_TOTAL_CHG_BAL_TIME_1_20);
    rslt = EE_LoadVar(SYSTEMST_TOTAL_CHG_BAL_TIME_21_40);
    rslt = EE_LoadVar(SYSTEMST_TOTAL_CHG_BAL_TIME_41_60);
    WdgIf_Trig();
    #if(MAX_CELL_NUM > 60)
    rslt = EE_LoadVar(SYSTEMST_TOTAL_CHG_BAL_TIME_61_80);    
    rslt = EE_LoadVar(SYSTEMST_TOTAL_CHG_BAL_TIME_81_100);
    rslt = EE_LoadVar(SYSTEMST_TOTAL_CHG_BAL_TIME_101_120);
    WdgIf_Trig();
    #endif
    
    rslt = EE_LoadVar(SYSTEMST_TOTAL_DCH_BAL_TIME_1_20);
    rslt = EE_LoadVar(SYSTEMST_TOTAL_DCH_BAL_TIME_21_40);
    rslt = EE_LoadVar(SYSTEMST_TOTAL_DCH_BAL_TIME_41_60);
    WdgIf_Trig();
    #if(MAX_CELL_NUM > 60)
    rslt = EE_LoadVar(SYSTEMST_TOTAL_DCH_BAL_TIME_61_80);    
    rslt = EE_LoadVar(SYSTEMST_TOTAL_DCH_BAL_TIME_81_100);
    rslt = EE_LoadVar(SYSTEMST_TOTAL_DCH_BAL_TIME_101_120);
    WdgIf_Trig();
    #endif
    
    #if(MAX_CELL_NUM > 120)
        rslt = EE_LoadVar(SYSTEMST_TOTAL_CHG_BAL_TIME_121_140);
        rslt = EE_LoadVar(SYSTEMST_TOTAL_CHG_BAL_TIME_141_160);
        rslt = EE_LoadVar(SYSTEMST_TOTAL_CHG_BAL_TIME_161_180);
        WdgIf_Trig();
        rslt = EE_LoadVar(SYSTEMST_TOTAL_DCH_BAL_TIME_121_140);
        rslt = EE_LoadVar(SYSTEMST_TOTAL_DCH_BAL_TIME_141_160);
        rslt = EE_LoadVar(SYSTEMST_TOTAL_DCH_BAL_TIME_161_180);
        WdgIf_Trig();
    #endif
    
    #if(MAX_CELL_NUM > 180)
        rslt = EE_LoadVar(SYSTEMST_TOTAL_CHG_BAL_TIME_181_200);
        rslt = EE_LoadVar(SYSTEMST_TOTAL_CHG_BAL_TIME_201_220);
        rslt = EE_LoadVar(SYSTEMST_TOTAL_CHG_BAL_TIME_221_240);
        WdgIf_Trig();
        rslt = EE_LoadVar(SYSTEMST_TOTAL_DCH_BAL_TIME_181_200);
        rslt = EE_LoadVar(SYSTEMST_TOTAL_DCH_BAL_TIME_201_220);
        rslt = EE_LoadVar(SYSTEMST_TOTAL_DCH_BAL_TIME_221_240);
        WdgIf_Trig();
    #endif
    
    #if(MAX_CELL_NUM > 240)
        rslt = EE_LoadVar(SYSTEMST_TOTAL_CHG_BAL_TIME_241_260);
        rslt = EE_LoadVar(SYSTEMST_TOTAL_CHG_BAL_TIME_261_280);
        rslt = EE_LoadVar(SYSTEMST_TOTAL_CHG_BAL_TIME_281_300);
        WdgIf_Trig();
        rslt = EE_LoadVar(SYSTEMST_TOTAL_DCH_BAL_TIME_241_260);
        rslt = EE_LoadVar(SYSTEMST_TOTAL_DCH_BAL_TIME_261_280);
        rslt = EE_LoadVar(SYSTEMST_TOTAL_DCH_BAL_TIME_281_300);
        WdgIf_Trig();
    #endif
    /* end of init total balance time */

    BalM_Ctrl.BalSt = ST_BAL_OFF;
    BalM_Ctrl.StartBal = BALM_FLAG_ENABLE;
    BalM_Ctrl.BalCtrl = BALM_FLAG_ENABLE;
    (void) rslt;
}

void BalM_ModuleCountCbk(void)
{
    (BalM_TmCnt.cnt < 255) ? (BalM_TmCnt.cnt++) : (BalM_TmCnt.cnt = 255);
    (BalM_TmCnt.ReceiveCmdCnt < 255) ? (BalM_TmCnt.ReceiveCmdCnt++) : (BalM_TmCnt.ReceiveCmdCnt = 255);
}

void BalM_ModuleCbk(void)
{
    if(BalM_TmCnt.cnt >= TM_MS(200))
    {
        BalM_TmCnt.cnt = 0;

        BalM_SendBalCmd();


        BalM_ProcBalTime();
    }   
}

static void BalM_ProcBalTime(void)
{
    GeneralReturnType rslt;
    uByte i;
    uByte data[2], dlen;
    uByte bmu_num;
    uByte bmu_no;
    uWord bat_num;
    uWord j;
    uWord k;
    uLWord lwtmp;
    uWord wtmp;

    BalM_BalCnt.TimeCnt10s++;
    BalM_BalCnt.TimeCnt5Min++;
    
    if(BalM_BalCnt.SysStartCnt < BALM_TIME_10MIN)
    {
       BalM_BalCnt.SysStartCnt++;
    }
    else
    {
       BalM_BalCnt.SysStartCnt = BALM_TIME_10MIN;
    }

    bmu_no = 0;
    rslt = SysConf_GetParaValueFromRam(SYSCONF_BMU_NUMBER, &bmu_num, &dlen);
    
    // need modify bat_num
    bat_num = Bat_GetBatNum();

    if(BalM_BalCnt.TimeCnt10s >= BALM_TIME_10S)
    {
        BalM_BalCnt.TimeCnt10s = 0;

        /* calc single balance time and remain balance time below */
        k = 0;
        lwtmp = 1;

        for(i = 1; i <= bmu_num; i++)
        {
            if(ST_BAL_ON == Bat_GetPackBalSt(i))
            {
                for(j = 0; j < bat_num; j++) 
                {
#ifdef BMS_ENABLE_SINGLE_BAL_TIME
                    if(DCH_BAL_ON == Bat_GetCellBalanceSt(i, j + 1)) // cell is discharge balance now
                    {
                        (void)Bat_UpdateCellDchBalanceTimeSingle(k + 1, (Bat_GetCellDchBalanceTimeSingle(k + 1) + 1));
                    }
                    else if(CHG_BAL_ON == Bat_GetCellBalanceSt(i, j + 1))  // cell is charge balance now
                    {
                        (void)Bat_UpdateCellChgBalanceTimeSingle(k + 1, (Bat_GetCellChgBalanceTimeSingle(k + 1) + 1));
                    }
#endif
                    k++;
                }
            }
        }
        /* end of calc single balance time and remain balance time */
    }

    /* calc total balance time below */
    k = 0;
    lwtmp = 1;

    for(i = 1; i <= bmu_num; i++)
    {
        if(ST_BAL_ON == Bat_GetPackBalSt(i))
        {
            for(j = 0; j < bat_num; j++)  // maybe need modify series num
            {
                if(DCH_BAL_ON == Bat_GetCellBalanceSt(i, j + 1)) // cell is discharge balance now
                {
                    BalM_BalCnt.TotalDchBalCnt[i][j]++;

                    if(BalM_BalCnt.TotalDchBalCnt[i][j] > BALM_TIME_10MIN)
                    {
                        BalM_BalCnt.TotalDchBalCnt[i][j] = 0;
                        (void)Bat_UpdateCellDchBalanceTimeTotal(k + 1, (Bat_GetCellDchBalanceTimeTotal(k + 1) + 1));
                        BalM_Ctrl.SaveFlag2 |= lwtmp << (uByte)(k / 20);
                    }
                }
                else if(CHG_BAL_ON == Bat_GetCellBalanceSt(i, j + 1))  // cell is charge balance now
                {
                    BalM_BalCnt.TotalChgBalCnt[i][j]++;

                    if(BalM_BalCnt.TotalChgBalCnt[i][j] > BALM_TIME_10MIN)
                    {
                        BalM_BalCnt.TotalChgBalCnt[i][j] = 0;
                        (void)Bat_UpdateCellChgBalanceTimeTotal(k + 1, (Bat_GetCellChgBalanceTimeTotal(k + 1) + 1));
                        BalM_Ctrl.SaveFlag3 |= lwtmp << (uByte)(k / 20);
                    }
                }
                k++;
            }
        }
    }
    /* end of calc total balance time */


    /* save balance time to EE every 5min */
    if((BalM_BalCnt.TimeCnt5Min >= BALM_TIME_5MIN)
      || (LVM_STATE_OFF == BmsSys_GetAwakeSt()) ) //|| POWEROFF
    {
        BalM_BalCnt.TimeCnt5Min = 0;

       /* save total balance time to EE */
       lwtmp = 1;
       for(i = 0; i < 15; i++)
        {
            if(0 != ((BalM_Ctrl.SaveFlag2 >> i) & 0x01))
            {
                wtmp = i + (uWord)SYSTEMST_TOTAL_DCH_BAL_TIME_1_20;
                rslt = EE_SaveVar(BOTHEE, (eSysConf_ParameterNOType)wtmp);
                BalM_Ctrl.SaveFlag2 &= (~(lwtmp << i));
            }

            if(0 != ((BalM_Ctrl.SaveFlag3 >> i) & 0x01))
            {
                wtmp = i + (uWord)SYSTEMST_TOTAL_CHG_BAL_TIME_1_20;
                rslt = EE_SaveVar(BOTHEE, (eSysConf_ParameterNOType)wtmp);
                BalM_Ctrl.SaveFlag3 &= (~(lwtmp << i));
            }
        }
        /* end of save total balance time to EE */

    }
    /* end of save balance time to EE */
    (void) rslt;
}

//************************************************************/
// Function:
// Trans para:
//************************************************************/
// BCU-C or BCU-D or BMU-R control balance below
static void BalM_SendBalCmd(void)
{
    uByte i;
    uByte btmp;
    uWord wtmp;
    uByte dlen;
    uByte bmu_no;
    uByte bmu_num;
    uByte chg_st;
    uWord current;
    uWord bms_hd_err;
    uLWord llwd;
    GeneralReturnType rslt;

    rslt = SysConf_GetParaValueFromRam(SYSCONF_BMU_NUMBER, &bmu_num, &dlen);
    bmu_no = 0;


    // get bms hardware error code
    bms_hd_err = (uWord)Err_GetHwErrLevel(ERR_HW_DETECT_CUR);
    bms_hd_err = (bms_hd_err << 2) + (uWord)Err_GetHwErrLevel(ERR_HW_DETECT_CELLV);
    bms_hd_err = (bms_hd_err << 2) + (uWord)Err_GetHwErrLevel(ERR_HW_CAN4);
    bms_hd_err = (bms_hd_err << 2) + (uWord)Err_GetHwErrLevel(ERR_HW_EE_CPU);
    bms_hd_err = (bms_hd_err << 2) + (uWord)Err_GetHwErrLevel(ERR_HW_EE_EXT);
    bms_hd_err = (bms_hd_err << 2) + (uWord)Err_GetHwErrLevel(ERR_HW_BAL_DCH);

    BalM_Ctrl.MaxV = Bat_GetMaxV();
    BalM_Ctrl.MinV = Bat_GetMinV();
    BalM_Ctrl.AvgV = Bat_GetAvgV();
    BalM_Ctrl.BatCap = Bat_GetRatedCapacityCalc();
    current = Bat_GetCurrent(AVGCUR);

    chg_st = Chg_GetChgSt();

    // judge battery status of "charge now"
    if( (CHG_ST_ON == chg_st) && (current <= CHG_CUR_2A)
     && (BALM_BAT_CHG_END != BalM_Ctrl.BatSt) 
     && (BALM_BAT_AFTER_CHG != BalM_Ctrl.BatSt) )
    {
        BalM_Ctrl.BatSt = BALM_BAT_CHG_NOW;
        BalM_BalCnt.AfterChgCnt = 0;
    }

    // judge battery status of "after charge"
    if((BALM_BAT_CHG_NOW == BalM_Ctrl.BatSt) || (BALM_BAT_CHG_END == BalM_Ctrl.BatSt))
    {
        if((current > CHG_CUR_2A) && (current <= DCH_CUR_10A))
        {
            if(BalM_BalCnt.AfterChgCnt > BALM_TIME_1MIN)
            {
                BalM_Ctrl.BatSt = BALM_BAT_AFTER_CHG;
            }
            else
            {
                BalM_BalCnt.AfterChgCnt++;
            }
        }
    }

    // judge battery status of "no charge"
    if(current > DCH_CUR_10A)
    {
        BalM_Ctrl.BatSt = BALM_BAT_NOT_CHG;
        BalM_BalCnt.AfterChgCnt = 0;
    }

    // check max volt
    if( (BalM_Ctrl.MaxV > BalM_ConfigPara.Max_v_Calc_Bal_Thd)  //3.4V
     && (current <= (OFFSET_CUR + (BalM_Ctrl.BatCap * 35 / 1000)))  // current <= 0.35C
     && (current >= (OFFSET_CUR - (BalM_Ctrl.BatCap * 35 / 1000))) )
    {
        if(BalM_BalCnt.VoltHighCnt < BALM_TIME_1MIN)
        {
            BalM_BalCnt.VoltHighCnt++;
        }
    }
    else
    {
        BalM_BalCnt.VoltHighCnt = 0;
    }

    // calc balance every 3min
    if(BalM_BalCnt.CalcBalCnt < BALM_TIME_3MIN)
    {
        BalM_BalCnt.CalcBalCnt++;
    }
        
    llwd = 0;   
    rslt = BmsSys_CalcStandbyTime(&llwd);

    if((BalM_ConfigPara.BalMode == BALM_MODE_D) ||(BalM_ConfigPara.BalMode == BALM_MODE_E))
    {      
        if((BalM_BalCnt.SysStartCnt < BALM_TIME_2S) && (BALM_FLAG_FAIL == BalM_Ctrl.BatStandByFlag))
        {
            if( (BalM_Ctrl.MinV > 0)
             && (BalM_Ctrl.MaxV < 4500)
             && (0 == bms_hd_err)
             && (RELAY_CLOSED != BatSys_GetPosRlySt())
             && (RELAY_CLOSED != BatSys_GetPreRlySt()) 
             && (ERR_OK == rslt)          
             && (BalM_ConfigPara.Bal_Ocv_Standby_Time <= llwd) )   
            {
                BalM_Ctrl.BatStandByFlag = BALM_FLAG_OK;
                BalM_Ctrl.BatSt = BALM_BAT_STANDBY;
                
                if(llwd > BALM_240_HOUR)  
                {
                    //BalM_Ctrl.ForceBal = BALM_FLAG_DISABLE; 
                    
                    BalM_Ctrl.TxBalCmd |= MASK_FORCE_BAL;
                    BalM_Ctrl.RxBalCmd &= (~MASK_FORCE_BAL);
                    BalM_Ctrl.BalSt = ST_BAL_OFF;
                    Bat_UpdateMainBalanceSt(ST_BAL_OFF);           
                }
                
                BalM_BalCnt.AfterChgCnt = 0;
                BalM_BalCnt.CalcBalCnt = 0;
                
                if(BalM_Ctrl.MinV > BalM_ConfigPara.Ocv_Calc_Bal_Thd)
                {
                    BalM_Ctrl.CalcBal = BALM_FLAG_ENABLE;
                }
                //BalM_Ctrl.BalCtrl = BALM_FLAG_DISABLE;

                // clear flag
                BalM_RxSla.StartBalRsltCnt = 0;
                BalM_RxSla.StopBalRsltCnt = 0;
                BalM_RxSla.CalcBalRsltCnt = 0;
                BalM_RxSla.ForceBalRsltCnt = 0;
                for(i = 0; i < bmu_num; i++)
                {
                    BalM_RxSla.StartBalRslt[i] = 0;
                    BalM_RxSla.StopBalRslt[i] = 0;
                    BalM_RxSla.CalcBalRslt[i] = 0;
                    BalM_RxSla.ForceBalRslt[i] = 0;
                }       
            }
        }
    }
    // calc balance when max volt large enough
    if(BalM_ConfigPara.BalMode != BALM_MODE_D)
    {      
        if( ((BALM_BAT_CHG_NOW == BalM_Ctrl.BatSt) || (BALM_BAT_AFTER_CHG == BalM_Ctrl.BatSt))
          && (BalM_BalCnt.VoltHighCnt >= BALM_TIME_1MIN) 	// volt is high last for 1min
          && (BalM_BalCnt.CalcBalCnt >= BALM_TIME_3MIN) )	// calc balance every 3min
        {
            BalM_BalCnt.CalcBalCnt = 0;
            BalM_Ctrl.CalcBal = BALM_FLAG_ENABLE;
            BalM_Ctrl.StartBal = BALM_FLAG_ENABLE;

            // clear flag
            BalM_RxSla.StopBalRsltCnt = 0;
            BalM_RxSla.CalcBalRsltCnt = 0;
            BalM_RxSla.StartBalRsltCnt = 0;
            for(i = 0; i < bmu_num; i++)
            {
                BalM_RxSla.StopBalRslt[i] = 0;
                BalM_RxSla.CalcBalRslt[i] = 0;
                BalM_RxSla.StartBalRslt[i] = 0;
            }
        }

        // calc balance when charge end
        //if( (CHG_ST_END == chg_st) && (BALM_FLAG_ENABLE == BalM_Ctrl.BalCtrl))
        if((CHG_STAGE_READY_TO_END == Chg_GetChgStage()) && (CHG_ST_ON == Chg_GetChgSt())
          && (BALM_FLAG_ENABLE == BalM_Ctrl.BalCtrl))
        {
            // judge battery status of "charge end"
            BalM_Ctrl.BatSt = BALM_BAT_CHG_END;
            BalM_BalCnt.AfterChgCnt = 0;
            BalM_BalCnt.CalcBalCnt = 0;
            BalM_Ctrl.CalcBal = BALM_FLAG_ENABLE;
            BalM_Ctrl.BalCtrl = BALM_FLAG_DISABLE;

            // clear flag
            BalM_RxSla.StopBalRsltCnt = 0;
            BalM_RxSla.CalcBalRsltCnt = 0;
            for(i = 0; i < bmu_num; i++)
            {
                BalM_RxSla.StopBalRslt[i] = 0;
                BalM_RxSla.CalcBalRslt[i] = 0;
            }
        }
    }
    // calc balance
    if((BALM_FLAG_ENABLE == BalM_Ctrl.CalcBal) && (0 == bms_hd_err) && (BalM_Ctrl.MinV >= BalM_ConfigPara.Min_v_Calc_Bal_Thd))
    {
        BalM_Ctrl.CalcBal = BALM_FLAG_DISABLE;
        BalM_Ctrl.TxBalCmd |= MASK_CAL_BAL;
        BalM_Ctrl.RxBalCmd &= (~MASK_CAL_BAL);
        BalM_Ctrl.BalSt = ST_BAL_OFF;
        Bat_UpdateMainBalanceSt(ST_BAL_OFF);
    }

    // start balance
    if( (BALM_FLAG_ENABLE == BalM_Ctrl.StartBal)
     &&((BalM_ConfigPara.BalMode != BALM_MODE_A) || ((BalM_ConfigPara.BalMode == BALM_MODE_A)
         && ((BALM_BAT_CHG_NOW == BalM_Ctrl.BatSt) || (BALM_BAT_AFTER_CHG == BalM_Ctrl.BatSt))))
     && (0 == (BalM_Ctrl.TxBalCmd & MASK_CAL_BAL))
     && (0 == (BalM_Ctrl.TxBalCmd & MASK_FORCE_BAL))
     && (0 == (BalM_Ctrl.TxBalCmd & MASK_STOP_BAL))
     && (BalM_Ctrl.MinV >= BalM_ConfigPara.Min_v_Start_Bal_Thd)
     && (0 == bms_hd_err) )
    {
        if(BalM_BalCnt.StartBalCnt >= BALM_TIME_5S)
        {
            BalM_BalCnt.StartBalCnt = 0;
            BalM_Ctrl.StartBal = BALM_FLAG_DISABLE;
            BalM_Ctrl.TxBalCmd |= MASK_START_BAL;
            BalM_Ctrl.RxBalCmd &= (~MASK_START_BAL);
         
            BalM_Ctrl.BalSt = ST_BAL_ON;
        
            Bat_UpdateMainBalanceSt(ST_BAL_ON);
        }
        else
        {
            BalM_BalCnt.StartBalCnt++;
        }
    }
    else
    {
        BalM_BalCnt.StartBalCnt = 0;
    }

    // stop balance
    if( (0 != bms_hd_err)
    || ((BalM_ConfigPara.BalMode == BALM_MODE_A) && (BALM_BAT_NOT_CHG == BalM_Ctrl.BatSt)) 
    || (BalM_Ctrl.MinV < BalM_ConfigPara.Min_v_Stop_Bal_Thd) ) 
    {
        BalM_Ctrl.TxBalCmd &= (~MASK_START_BAL);
        BalM_Ctrl.TxBalCmd &= (~MASK_CAL_BAL);
        BalM_Ctrl.TxBalCmd &= (~MASK_FORCE_BAL);

        if(ST_BAL_ON == BalM_Ctrl.BalSt)
        {
            BalM_Ctrl.TxBalCmd |= MASK_STOP_BAL;
            BalM_Ctrl.RxBalCmd &= (~MASK_STOP_BAL);

            BalM_Ctrl.BalSt = ST_BAL_OFF;
            
            Bat_UpdateMainBalanceSt(ST_BAL_OFF);
        }
    }

    // re enable send start balance cmd
    if( (BALM_FLAG_ENABLE == BalM_Ctrl.BalCtrl)  
     && (0 == (BalM_Ctrl.TxBalCmd & MASK_CAL_BAL))
     && (0 == (BalM_Ctrl.TxBalCmd & MASK_STOP_BAL))
     && (0 == (BalM_Ctrl.TxBalCmd & MASK_FORCE_BAL))
     && (0 == bms_hd_err)
     && (ST_BAL_OFF == BalM_Ctrl.BalSt)
     && (BalM_Ctrl.MinV >= BalM_ConfigPara.Min_v_Start_Bal_Thd) )
    {
        BalM_Ctrl.StartBal = BALM_FLAG_ENABLE;
    }

    // receive bmu ack of bal cmd
    BalM_RxSla.CalcBalRsltCnt = 0;
    BalM_RxSla.StartBalRsltCnt = 0;
    BalM_RxSla.StopBalRsltCnt = 0;
    BalM_RxSla.ForceBalRsltCnt = 0;

    for(i = 0; i < bmu_num; i++)
    {
        if(0 != BalM_RxSla.CalcBalRslt[i])
        {
            BalM_RxSla.CalcBalRsltCnt++;
        }

        if(0 != BalM_RxSla.StartBalRslt[i])
        {
            BalM_RxSla.StartBalRsltCnt++;
        }

        if(0 != BalM_RxSla.StopBalRslt[i])
        {
            BalM_RxSla.StopBalRsltCnt++;
        }

        if(0 != BalM_RxSla.ForceBalRslt[i])
        {
            BalM_RxSla.ForceBalRsltCnt++;
        }
    }

    if((BalM_Ctrl.RxBalCmd & MASK_FORCE_BAL) != MASK_FORCE_BAL)
    {
        btmp = BALM_FLAG_OK;

        // all bmu has ack
        if(BalM_RxSla.ForceBalRsltCnt >= bmu_num)
        {
            BalM_RxSla.ForceBalRsltCnt = 0;

            for(i = 0; i < bmu_num; i++)
            {
                btmp &= BalM_RxSla.ForceBalRslt[i];
            }

            // stop send cmd of 'force bal'
            if((BALM_FLAG_OK == btmp) && (bmu_num != 0))
            {
                BalM_Ctrl.RxBalCmd |= MASK_FORCE_BAL;
                BalM_Ctrl.TxBalCmd &= (~MASK_FORCE_BAL);

                for(i = 0; i < bmu_num; i++)
                {
                    BalM_RxSla.ForceBalRslt[i] = 0;
                }
            }

        }
    }

    if((BalM_Ctrl.RxBalCmd & MASK_CAL_BAL) != MASK_CAL_BAL)
    {
        btmp = BALM_FLAG_OK;

        // all bmu has ack
        if(BalM_RxSla.CalcBalRsltCnt >= bmu_num)
        {
            BalM_RxSla.CalcBalRsltCnt = 0;

            for(i = 0; i < bmu_num; i++)
            {
                if(0 == BalM_RxSla.CalcBalRslt[i])
                {
                    btmp = 0;
                }
            }

            // stop send cmd of 'calc bal'
            if((btmp != 0) && (bmu_num != 0))
            {
                BalM_Ctrl.RxBalCmd |= MASK_CAL_BAL;
                BalM_Ctrl.TxBalCmd &= (~MASK_CAL_BAL);

                for(i = 0; i < bmu_num; i++)
                {
                    BalM_RxSla.CalcBalRslt[i] = 0;
                }
            }

        }
    }

    if((BalM_Ctrl.RxBalCmd & MASK_START_BAL) != MASK_START_BAL)
    {
        btmp = BALM_FLAG_OK;

        // all bmu has ack
        if(BalM_RxSla.StartBalRsltCnt >= bmu_num)
        {
            BalM_RxSla.StartBalRsltCnt = 0;

            for(i = 0; i < bmu_num; i++)
            {
                btmp &= BalM_RxSla.StartBalRslt[i];
            }

            // stop send cmd of 'start bal'
            if((BALM_FLAG_OK == btmp) && (bmu_num != 0))
            {
                BalM_Ctrl.RxBalCmd |= MASK_START_BAL;
                BalM_Ctrl.TxBalCmd &= (~MASK_START_BAL);

                for(i = 0; i < bmu_num; i++)
                {
                    BalM_RxSla.StartBalRslt[i] = 0;
                }
            }

        }
    }

    if((BalM_Ctrl.RxBalCmd & MASK_STOP_BAL) != MASK_STOP_BAL)
    {
        btmp = BALM_FLAG_OK;

        // all bmu has ack
        if(BalM_RxSla.StopBalRsltCnt >= bmu_num)
        {
            BalM_RxSla.StopBalRsltCnt = 0;

            for(i = 0; i < bmu_num; i++)
            {
                btmp &= BalM_RxSla.StopBalRslt[i];
            }

            if((BALM_FLAG_OK == btmp) && (bmu_num != 0))
            {
                // stop send cmd of 'stop bal'
                BalM_Ctrl.RxBalCmd |= MASK_STOP_BAL;
                BalM_Ctrl.TxBalCmd &= (~MASK_STOP_BAL);

                for(i = 0; i < bmu_num; i++)
                {
                    BalM_RxSla.StopBalRslt[i] = 0;
                }
            }

        }
    }
    // end of receive bmu ack of bal cmd

    // send balance command, need modify
    if(BalM_Ctrl.TxBalCmd & MASK_FORCE_BAL)
    {
        BalM_SendCmd_ForceBal();
       
    }
    else if(BalM_Ctrl.TxBalCmd & MASK_CAL_BAL)
    {
        BalM_SendCmd_CalcBal();
      
    }
    else if(BalM_Ctrl.TxBalCmd & MASK_START_BAL)
    {
        BalM_SendCmd_StartBal();
       
    }
    else if(BalM_Ctrl.TxBalCmd & MASK_STOP_BAL)
    {
        BalM_SendCmd_StopBal();
        
    }
    else
    {
        // nop
    }
    // end of send balance command
}
// end of BCU-C or BCU-D or BMU-R control balance  


static void BalM_SendCmd_CalcBal(void)
{
    uWord rslt;
    uWord wtmp;
    uByte msg_data[10];
    uByte bmu_num;
    uByte dlen;
    
    rslt = SysConf_GetParaValueFromRam(SYSCONF_BMU_NUMBER, &bmu_num, &dlen);

    if(0 != bmu_num)
    {
        // use can send msg
        msg_data[0] = BalM_Ctrl.BatSt;
        // battery average volt
        wtmp = BalM_Ctrl.AvgV; // 0.001V unit
        msg_data[1] = ((wtmp & 0xFF00) >> 8);
        msg_data[2] = (wtmp & 0x00FF);
        // battery min volt
        wtmp = BalM_Ctrl.MinV; // 0.001V unit
        msg_data[3] = ((wtmp & 0xFF00) >> 8);
        msg_data[4] = (wtmp & 0x00FF);
        // battery max volt
        wtmp = BalM_Ctrl.MaxV; // 0.001V unit
        msg_data[5] = ((wtmp & 0xFF00) >> 8);
        msg_data[6] = (wtmp & 0x00FF);
        // battery capacity
        wtmp = (uWord)(BalM_Ctrl.BatCap / 100); // 0.1AH unit
        msg_data[7] = ((wtmp & 0xFF00) >> 8);
        msg_data[8] = (wtmp & 0x00FF);
        msg_data[9] = BalM_ConfigPara.BalMode;//BalM_Ctrl.BalMode;

        IntCan_SendBalCmd(CAL_BAL, &msg_data[0], 10);
    }
}

static void BalM_SendCmd_StartBal(void)
{
    uWord rslt;
    uByte msg_data[2];
    uByte bmu_num;
    uByte dlen;
    
    rslt = SysConf_GetParaValueFromRam(SYSCONF_BMU_NUMBER, &bmu_num, &dlen);

    if(0 != bmu_num)
    {
        // use can send msg
        msg_data[0] = 1;
        IntCan_SendBalCmd(START_BAL, &msg_data[0], 1);
    }
}

static void BalM_SendCmd_StopBal(void)
{
    uWord rslt;
    uByte msg_data[2];
    uByte bmu_num;
    uByte dlen;

    rslt = SysConf_GetParaValueFromRam(SYSCONF_BMU_NUMBER, &bmu_num, &dlen);

    if(0 != bmu_num)
    {
        // use can send msg
        msg_data[0] = 1;
        IntCan_SendBalCmd(STOP_BAL, &msg_data[0], 1);
    }
}

static void BalM_SendCmd_ForceBal(void)
{
    uWord rslt;
    uByte msg_data[2];
    uByte bmu_num;
    uByte dlen;

    rslt = SysConf_GetParaValueFromRam(SYSCONF_BMU_NUMBER, &bmu_num, &dlen);

    if(0 != bmu_num)
    {
        // use can send msg
        msg_data[0] = 0xFE;
        IntCan_SendBalCmd(FORCE_BAL, &msg_data[0], 1);
    }
}



//************************************************************/
// Function:
// Trans para:
//************************************************************/
void BalM_UpdateDataFromCan(uByte bmu_no, uByte cmd, uByte *p_data, uByte len)
{
    uWord wtmp;

    if((bmu_no > 0) && (bmu_no <= MAX_PACK_NUM))
    {
        if((uByte)CAL_BAL == cmd)
        {
            BalM_RxSla.CalcBalRslt[bmu_no- 1] = *p_data;
        }
        else if((uByte)START_BAL == cmd)
        {
            BalM_RxSla.StartBalRslt[bmu_no- 1] = *p_data;
        }
        else if((uByte)STOP_BAL == cmd)
        {
            BalM_RxSla.StopBalRslt[bmu_no- 1] = *p_data;
        }
        else if((uByte)FORCE_BAL == cmd)
        {
            BalM_RxSla.ForceBalRslt[bmu_no- 1] = *p_data;
        }
        else
        {
        }
    }
}

//************************************************************/
// Function:
// Trans para:
//************************************************************/
static void BalM_ReceiveBalCmd(void)
{
    uByte bmu_no, dlen;
    uWord i;
    uByte msg_data[2];
    GeneralReturnType rslt;
    
    (void) rslt;
}

//************************************************************/
// Function:
// Trans para:
//************************************************************/


static uWord BalM_SaveBalTime(void)
{
    uWord rslt = ERR_OK;
    
    return rslt;
}

#endif // #ifdef USE_BALM_E

