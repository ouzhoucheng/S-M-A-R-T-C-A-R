#include "car_global.h"
#include "data.h"
void data_output()

{

    if (mycar.status)
    {
        if (outdata.left_pwm >= 0)
        {
            pwm_duty(PWM1_MODULE3_CHB_C17, outdata.left_pwm);
            pwm_duty(PWM1_MODULE3_CHA_C16, 0);
        }
        else
        {
            pwm_duty(PWM1_MODULE3_CHA_C16, -outdata.left_pwm);
            pwm_duty(PWM1_MODULE3_CHB_C17, 0);
        }

        //    if(outdata.right_pwm >= 0)
        //    {
        //      pwm_duty(PWM2_MODULE3_CHB_C19, outdata.right_pwm);
        //      pwm_duty(PWM2_MODULE3_CHA_C18, 0);
        //    }
        //    else
        //    {
        //      pwm_duty(PWM2_MODULE3_CHA_C18, -outdata.right_pwm);
        //      pwm_duty(PWM2_MODULE3_CHB_C19, 0);
        //    }

        if (outdata.right_pwm >= 0)
        {
            pwm_duty(PWM2_MODULE3_CHA_C18, outdata.right_pwm);
            pwm_duty(PWM2_MODULE3_CHB_C19, 0);
        }
        else
        {
            pwm_duty(PWM2_MODULE3_CHB_C19, -outdata.right_pwm);
            pwm_duty(PWM2_MODULE3_CHA_C18, 0);
        }
    }
    else
    {
        pwm_duty(PWM1_MODULE3_CHB_C17, setpara.testL > 0 ? setpara.testL : 0);
        pwm_duty(PWM1_MODULE3_CHA_C16, setpara.testL < 0 ? -setpara.testL : 0);
        pwm_duty(PWM2_MODULE3_CHA_C18, setpara.testR > 0 ? setpara.testR : 0);
        pwm_duty(PWM2_MODULE3_CHB_C19, setpara.testR < 0 ? -setpara.testR : 0);
    }
    pwm_duty(PWM2_MODULE2_CHB_C11, outdata.steer_pwm);
}