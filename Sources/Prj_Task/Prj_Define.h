#ifndef PRJ_DEFINE_H
#define PRJ_DEFINE_H

#include "Elock.h"
#include "GeneralTypeDef.h"


/*            define for SOC         */
/*            define for SOC         */
/*            define for SOC         */
#define SOC_FUN_ENABLE   1        //���ܿ���ʹ��
#define SOC_FUN_DISABLE  0        //���ܿ��عر�

#define OCV_INIT_ESTMAT   SOC_FUN_DISABLE //�����������ܿ���
#define CHG_END_ESTMAT    SOC_FUN_ENABLE //�����������ܿ���
#define LMT_LOW_ESTMAT    SOC_FUN_DISABLE //���޵Ͷ��������ܿ���
#define LMT_HIGH_ESTMAT   SOC_FUN_DISABLE  //���޸߶��������ܿ���
#define SOC_SMOOTH        SOC_FUN_DISABLE  //ƽ���������ܿ���

#define DELTAV_SOC_LESS_985  200U   // unit 1mV  //���þ������ѹ�㵥���ѹ���ڴ�ֵʱ��SOC������98.5%
#define DELTAV_SOC_LESS_995  100U   // unit 1mV  //���þ������ѹ�㵥���ѹ���ڴ�ֵʱ��SOC������99.5%

#define DELTA_MIN_V          1000U  // unit 1mV //ƽ����ѹ����͵����ѹ���ڴ�ֵʱ��SOC����ʱ�ж��������͵�ѹ��Ϊ�ж�������Ҫ��ƽ����ѹ��Ϊ�ж�����

#define OCV_STANDBY_TIME     3600U // unit 1s   //�趨��������ʱ����ʱ�䣬��λ1S
#define OCV_LINEAR_RANGE_L1  0     // unit 1mV  //�趨�������������ѹ���Զ���ʼ��ѹ
#define OCV_LINEAR_RANGE_H1  4200U // unit 1mV  //�趨�������������ѹ���Զν�����ѹ
#define OCV_LINEAR_RANGE_L2  0     // unit 1mV  //�趨�������������ѹ���Զ���ʼ��ѹ
#define OCV_LINEAR_RANGE_H2  4200 // unit 1mV  //�趨�������������ѹ���Զν�����ѹ

#define AVGV_CHG_END         4000//4120U // unit 1mV //�趨��������ʱƽ����ѹ��Ҫ�ﵽ������

#define MAX_CELLV_LMT        4200U // unit 1mV   //�趨���޸߶������ĵ����ѹ��ֵ����ֵ������ڵ����ѹ��������������ֵһ��
#define MIN_CELLV_LMT        2500U // unit 1mV   //�趨���޵Ͷ������ĵ����ѹ��ֵ����ֵ������ڵ����ѹ��������������ֵһ��

#define SOC_MAX_SOC_POINT  21U //�趨��������������ݱ��SOC�����
#define SOC_MAX_T_POINT    4U  //�趨��������������ݱ���¶ȵ����


/*            end of define for SOC        */
/*            end of define for SOC        */
/*            end of define for SOC        */



/*            define for SOF         */
/*            define for SOF         */
/*            define for SOF         */
#define SOF_FUN_ENABLE      1        //���ܿ���ʹ��
#define SOF_FUN_DISABLE     0        //���ܿ��عر�

#define SOF_DCH_CUR_VOLT_MODIFY   SOF_FUN_DISABLE   //�����������ŵ����ʱ���Ǹ��ݵ����ѹ�������Ĺ��ܿ���
#define SOF_DCH_CUR_ERR_MODIFY    SOF_FUN_DISABLE   //�����������ŵ����ʱ���Ǹ��ݹ��������������Ĺ��ܿ���
#define SOF_FD_CUR_VOLT_MODIFY    SOF_FUN_DISABLE   //������������������ʱ���Ǹ��ݵ����ѹ�������Ĺ��ܿ���
#define SOF_FD_CUR_ERR_MODIFY     SOF_FUN_DISABLE   //������������������ʱ���Ǹ��ݹ��������������Ĺ��ܿ���
#define SOF_PLUSE_CUR_MODIFY      SOF_FUN_DISABLE   //���������������������ܿ���
#define SOF_CALC_MAX_POWER        SOF_FUN_ENABLE    //������������ʣ��������ʡ����幦�ʣ����ܿ���
#define SOF_USER_FUN              SOF_FUN_ENABLE    //�û��Զ������ŵ硢�������������ʹ���

#define SOF_MAX_T_POINT      12    //���ʵ�����ݱ���¶ȵ����
#define SOF_MAX_SOC_POINT    12    //���ʵ�����ݱ��SOC�����
#define SOF_MAX_VOLT_POINT   4     //���㹦��ʱ���ǵ�ѹ�����ĵ�ѹ���ѹ����

#define SOF_CONST_DCHCUR_SMOOTH_UNIT       10    // 0.1A/10ms, unit 0.1A   //����������ŵ������ӵ�ǰ����ֵƽ��������Ŀ��ֵ���ٶ�
#define SOF_CONST_FEEDBACKCUR_SMOOTH_UNIT  10    // 0.1A/10ms, unit 0.1A   //�������������������ӵ�ǰ����ֵƽ��������Ŀ��ֵ���ٶ�
#define SOF_PULSE_DCHCUR_SMOOTH_UNIT       10    // 0.1A/10ms, unit 0.1A   //����������ŵ������ӵ�ǰ����ֵƽ��������Ŀ��ֵ���ٶ�
#define SOF_PULSE_FEEDBACKCUR_SMOOTH_UNIT  10    // 0.1A/10ms, unit 0.1A   //�������������������ӵ�ǰ����ֵƽ��������Ŀ��ֵ���ٶ�

#define SOF_CONST_DCHPOW_SMOOTH_UNIT       10    // 0.1kw/10ms, unit 0.1kw //����������ŵ繦�ʺ�ӵ�ǰ����ֵƽ��������Ŀ��ֵ���ٶ�
#define SOF_CONST_FEEDBACKPOW_SMOOTH_UNIT  10    // 0.1kw/10ms, unit 0.1kw //����������������ʺ�ӵ�ǰ����ֵƽ��������Ŀ��ֵ���ٶ�
#define SOF_PULSE_DCHPOW_SMOOTH_UNIT       10    // 0.1kw/10ms, unit 0.1kw //����������ŵ繦�ʺ�ӵ�ǰ����ֵƽ��������Ŀ��ֵ���ٶ�
#define SOF_PULSE_FEEDBACKPOW_SMOOTH_UNIT  10    // 0.1kw/10ms, unit 0.1kw //����������������ʺ�ӵ�ǰ����ֵƽ��������Ŀ��ֵ���ٶ�

#define SOF_MODIFY_CELLV_LOW_L1    50    //�����ѹ����һ�����ϵĽ��ŵ繦�ʰٷֱ�ֵ�趨
#define SOF_MODIFY_CELLV_LOW_L2    0     //�����ѹ���Ͷ������ϵĽ��ŵ繦�ʰٷֱ�ֵ�趨
#define SOF_MODIFY_SUMV_LOW_L1     50    //�ܵ�ѹ����һ�����ϵĽ��ŵ繦�ʰٷֱ�ֵ�趨
#define SOF_MODIFY_SUMV_LOW_L2     0     //�ܵ�ѹ���Ͷ������ϵĽ��ŵ繦�ʰٷֱ�ֵ�趨
#define SOF_MODIFY_DCHCUR_HIGH_L1  90    //�ŵ��������һ�����ϵĽ��ŵ繦�ʰٷֱ�ֵ�趨
#define SOF_MODIFY_DCHCUR_HIGH_L2  80    //�ŵ�������߶������ϵĽ��ŵ繦�ʰٷֱ�ֵ�趨

#define SOF_MODIFY_CELLV_HIGH_L1   50   //�����ѹ����һ�����ϵĽ��������ʰٷֱ�ֵ�趨
#define SOF_MODIFY_CELLV_HIGH_L2   0    //�����ѹ���߶������ϵĽ��������ʰٷֱ�ֵ�趨
#define SOF_MODIFY_SUMV_HIGH_L1    50   //�ܵ�ѹ����һ�����ϵĽ��������ʰٷֱ�ֵ�趨
#define SOF_MODIFY_SUMV_HIGH_L2    0    //�ܵ�ѹ���߶������ϵĽ��������ʰٷֱ�ֵ�趨
#define SOF_MODIFY_FDCUR_HIGH_L1   90   //������������һ�����ϵĽ��������ʰٷֱ�ֵ�趨
#define SOF_MODIFY_FDCUR_HIGH_L2   80   //�����������߶������ϵĽ��������ʰٷֱ�ֵ�趨

#define SOF_MODIFY_DETVOLT_L1      90   //ѹ�����һ�����Ͻ��ŵ硢�������ʰٷֱ�ֵ�趨
#define SOF_MODIFY_DETVOLT_L2      80   //ѹ�����������Ͻ��ŵ硢�������ʰٷֱ�ֵ�趨
#define SOF_MODIFY_DETTEMP_L1      90   //�²����һ�����Ͻ��ŵ硢�������ʰٷֱ�ֵ�趨
#define SOF_MODIFY_DETTEMP_L2      80   //�²����������Ͻ��ŵ硢�������ʰٷֱ�ֵ�趨
#define SOF_MODIFY_INTCAN_ERR      0    //�ڲ�CANͨ�Ź��Ͻ��ŵ硢�������ʰٷֱ�ֵ�趨

/*            end of define for SOF        */
/*            end of define for SOF        */
/*            end of define for SOF        */



/*            define for CHG         */
/*            define for CHG         */
/*            define for CHG         */
#define ELOCK_TEMP_POINT  166     //������NTC�¶ȵ����
#define ELOCK_TEMP_POINT1  166     //������NTC�¶ȵ����
//#define ELOCK_INIT_CMD()  ELOCK_LEFT_UNLOCK_FD_HIGH()  //�ϵ翪ʼ���Ƶ���������
#define CHGM_ERR_MAX_CELLV      4200     /* 3.7V */    //�趨���������ߵ�ѹ��ֵ����ߵ����ѹ���ڴ�ֵ���������жϲ�������
#define CHGM_ERR_MIN_CELLV      1000   /* 1V*/   //�趨���������͵�ѹ��ֵ����͵����ѹС�ڴ�ֵ���������жϲ�������

#define CHGM_ERR_MAX_SUMV       4030     /* 390V */ //�趨�����������ܵ�ѹ��ֵ���ܵ�ѹ���ڴ�ֵ���������жϲ�������
#define CHGM_ERR_MIN_SUMV       1000     /* 100V */ //�趨�����������ܵ�ѹ��ֵ���ܵ�ѹС�ڴ�ֵ���������жϲ�������

#define CHGM_ERR_MAX_TEMPER     105    /* 60'C, *///�趨�����������¶���ֵ������¶ȴ��ڴ�ֵ���������жϲ�������
#define CHGM_ERR_MIN_TEMPER     20     /* -10'C,*/ //�趨�����������¶���ֵ������¶�С�ڴ�ֵ���������жϲ�������

#define MINV_DOWN_CC_CUR   2000  /* 2.0V */   //�趨����͵����ѹС�ڴ��趨ֵʱ����������������������ԭ�������ϼ��룬Ҳ���ǵ�ѹ̫�ͣ�����С�������
#define MAXV_DOWN_CC_CUR   4030  //3600  /* 3.6V *///�趨�����ʱ��С��3���ӵ�����ߵ����ѹ�Ѿ��������趨ֵ���ж�Ϊ�����󷴸���磬�ӿ콵�����Է����ֹ������

#define CHG_DET_LIMIT_T    //�������ж��ʲ�������ʱ�Ƿ��ж��¶ȹ��ߺ͹��������������Ĺ��ܿ���
#define CHG_DET_LOW_V      //�������ж��ʲ�������ʱ�Ƿ��жϵ����ѹ����ѹ�����������������Ĺ��ܿ���
#define CHG_DET_GB_PLUG_CUR //���㽻��������������ʱ�Ƿ���CC��CP�Ĺ��ܿ���

#define CHG_RELAY_CTRL //�жϲ�������ʱ�Ƿ���Ҫ�Ͽ��̵����жϱ�־���ܿ���


#define CHGM_JUDGE_CUR          300                  /* 30 A*/ //�趨�����������������ڴ�ֵʱ���жϳ������ı�׼Ϊ�����������������*1.5��
#define CHGM_ERR_MAX_CUR        (OFFSET_CUR - 450)   /* 45A */ //�趨��������������С�������趨ֵʱ���жϳ�������׼ͳһΪ��ǰ�������ڴ��趨ֵ
#define CHGM_DET_START_CHG_CUR  (OFFSET_CUR - 20)    /* 2A */ //�趨������ʱ�����С����ֵ��Ҳ����ֻ�е������ڴ�ֵ����Ϊ���г����̣�������ʱ��

/*            end of define for CHG        */
/*            end of define for CHG        */
/*            end of define for CHG        */

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

#pragma DATA_SEG DEFAULT

#endif //#define PRJ_DEFINE_H