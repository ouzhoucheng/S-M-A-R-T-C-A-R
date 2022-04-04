#include "data.h"
#include "data_process.h"
// char distanceHZ = 0;
float SpeedK;
void data_process()
{
    /* SteerPWM Calculation */
    if (mycar.status != 3)
    {
        if (watch.InLoop)
        {
            outdata.steer_pwm = loop_steer_cal();
        }
        else if(mycar.status != 4)
        {
            outdata.steer_pwm = cam_steer_cal();
        }
        else if(mycar.status == 4 )
        {
            // SpeedK = (indata.target - indata.journey) / 30.0;
            if( flag.DetectMode == ART_wait   || 
                flag.DetectMode == ART_animal || 
                flag.DetectMode == ART_fruit  )
            {
                outdata.steer_pwm = setpara.SteerMid;
                indata.setspeed = 0;
            }
            else if(flag.DetectMode == ART_num_even     || 
                    flag.DetectMode == ART_num_odd_back )
            {
                outdata.steer_pwm = cam_steer_cal();
                indata.setspeed = setpara.DectectSpeed;
                // SpeedK = (indata.target - indata.journey) / 30.0;
                // indata.setspeed = SpeedK * setpara.DectectSpeed;
                // indata.setspeed = _LIMIT(indata.setspeed, 5, setpara.DectectSpeed);
            }
            else if(flag.DetectMode == ART_num_odd       ||
                    flag.DetectMode == ART_num_even_back )
            {
                outdata.steer_pwm = setpara.SteerMid;
                indata.setspeed = -setpara.DectectSpeed;
                // SpeedK = (indata.target - indata.journey) / 30.0;
                // indata.setspeed = SpeedK * setpara.DectectSpeed;
                // indata.setspeed = _LIMIT(indata.setspeed, -setpara.DectectSpeed, -5);
            }
            else if(flag.DetectMode == ART_tag_even     || 
                    flag.DetectMode == ART_tag_odd_back )
            {
                outdata.steer_pwm = cam_steer_cal();
                indata.setspeed = setpara.DectectSpeed10;
                // SpeedK = (indata.target - indata.journey) / 10.0;
                // indata.setspeed = SpeedK * setpara.DectectSpeed;
                // indata.setspeed = _LIMIT(indata.setspeed, 5, setpara.DectectSpeed);
            }
            else if(flag.DetectMode == ART_tag_odd     || 
                    flag.DetectMode == ART_tag_even_back )
            {
                outdata.steer_pwm = setpara.SteerMid;
                indata.setspeed = -setpara.DectectSpeed10;
                // SpeedK = (indata.target - indata.journey) / 10.0;
                // indata.setspeed = SpeedK * setpara.DectectSpeed;
                // indata.setspeed = _LIMIT(indata.setspeed, -setpara.DectectSpeed, -5);
            }
        }

        if (mycar.status != 2 && mycar.status != 4 &&\
                ( watch.InLoop == 2 || watch.InLoop == 6 || watch.InLoop == 3 || watch.InLoop == 7))
        {
            // indata.setspeed = speed_cal();
            indata.setspeed = speed_cal() +_LIMIT(setpara.SpeedUP - 1.0 * abs(outdata.steer_pwm - setpara.SteerMid) / setpara.MaxTurn * (setpara.SpeedUP - setpara.SpeedDOWN) + 1.0 * abs(indata.YawAngle) / 265 * (setpara.SpeedUP - setpara.SpeedDOWN) / 2, 0, setpara.SpeedUP)\
                                + _LIMIT(indata.PitchAngle * setpara.SlopeDown, -100, 0);
        }
        else if (mycar.status != 2 && mycar.status != 4)
        {
            // indata.setspeed = speed_cal();
            indata.setspeed = speed_cal() + setpara.SpeedUP - 1.0 * abs(outdata.steer_pwm - setpara.SteerMid) / setpara.MaxTurn * (setpara.SpeedUP - setpara.SpeedDOWN)\
                                + _LIMIT(indata.PitchAngle * setpara.SlopeDown, -100, 0);;
        }
    }

    indata.diff_speedL = indata.setspeed * (1 + (TREAD * tan((float)(setpara.SteerMid - outdata.steer_pwm) * PI / 1800) / WHEEL_DISTANCE * setpara.diffK / 100));
    indata.diff_speedR = indata.setspeed * (1 - (TREAD * tan((float)(setpara.SteerMid - outdata.steer_pwm) * PI / 1800) / WHEEL_DISTANCE * setpara.diffK / 100));

    if(mycar.status == 2)
    {
        outdata.left_pwm =  Motor_PID_CalL(indata.speedL,0);
        outdata.right_pwm = Motor_PID_CalR(indata.speedR,0);
    }
    else if(mycar.status == 1 || mycar.status == 3)
    {
        outdata.left_pwm = left_pwm_cal(indata.speedL, indata.diff_speedL);
        outdata.right_pwm = right_pwm_cal(indata.speedR, indata.diff_speedR);
    }
    else if(mycar.status == 4)
    {
        outdata.left_pwm = Detect_left_pwm_cal(indata.speedL, indata.diff_speedL);
        outdata.right_pwm = Detect_right_pwm_cal(indata.speedR, indata.diff_speedR);
    }
    else
    {
        outdata.left_pwm  =  left_pwm_cal(indata.speedL, setpara.testL);
        outdata.right_pwm = right_pwm_cal(indata.speedR, setpara.testR);
    }
}

int left_pwm_cal(int input_speed, int setspeed)
{
    static int poweroutL, errorL, last_errorL, last_last_errorL;

    errorL = setspeed - input_speed;
    int d_error = errorL - last_errorL;
    int dd_error = -2 * last_errorL + errorL + last_last_errorL;
    poweroutL += setpara.SpeedKp * d_error + setpara.SpeedKi * errorL + setpara.SpeedKd * dd_error;
    last_last_errorL = last_errorL;
    last_errorL = errorL;

    poweroutL = _LIMIT(poweroutL, -49000, 49000);

    return poweroutL;
}

int right_pwm_cal(int input_speed, int setspeed)
{
    static int poweroutR, errorR, last_errorR, last_last_errorR;

    errorR = setspeed - input_speed;
    int d_error = errorR - last_errorR;
    int dd_error = -2 * last_errorR + errorR + last_last_errorR;
    poweroutR += setpara.SpeedKp * d_error + setpara.SpeedKi * errorR + setpara.SpeedKd * dd_error;
    last_last_errorR = last_errorR;
    last_errorR = errorR;

    poweroutR = _LIMIT(poweroutR, -49000, 49000);

    return poweroutR;
}

int Detect_left_pwm_cal(int input_speed, int setspeed)
{
    static int poweroutL, errorL, last_errorL, last_last_errorL;
    errorL = setspeed - input_speed;
    int d_error = errorL - last_errorL;
    int dd_error = -2 * last_errorL + errorL + last_last_errorL;
    poweroutL += setpara.DsP * d_error + setpara.DsI * errorL + setpara.DsD * dd_error;
    last_last_errorL = last_errorL;
    last_errorL = errorL;
    poweroutL = _LIMIT(poweroutL, -49000, 49000);
    return poweroutL;
}

int Detect_right_pwm_cal(int input_speed, int setspeed)
{
    static int poweroutR, errorR, last_errorR, last_last_errorR;
    errorR = setspeed - input_speed;
    int d_error = errorR - last_errorR;
    int dd_error = -2 * last_errorR + errorR + last_last_errorR;
    poweroutR += setpara.DsP * d_error + setpara.DsI * errorR + setpara.DsD * dd_error;
    last_last_errorR = last_errorR;
    last_errorR = errorR;
    poweroutR = _LIMIT(poweroutR, -49000, 49000);
    return poweroutR;
}

int cam_steer_cal()
{
    int pwmout;
    static int16_t dir_h[10] = {0};
    static int16_t ddir_h[10] = {0};

    pwmout = setpara.SteerMid - setpara.CamKp * indata.cam_dir / 100 - setpara.CamKd * indata.cam_ddir / 10;
    pwmout = _LIMIT(pwmout, setpara.SteerMid - setpara.MaxTurn, setpara.SteerMid + setpara.MaxTurn);

    return pwmout;
}

int loop_steer_cal()
{
    int pwmout;
    static int16_t dir_h[10] = {0};
    static int16_t ddir_h[10] = {0};

    pwmout = setpara.SteerMid - setpara.CamLoopKp * indata.cam_dir / 100 - setpara.CamLoopKd * indata.cam_ddir / 10;
    pwmout = _LIMIT(pwmout, setpara.SteerMid - setpara.MaxTurn, setpara.SteerMid + setpara.MaxTurn);

    return pwmout;
}

int speed_cal()
{
    static int powerout = 0;
    // if (flag.SLOPE == 1)
    //     powerout = setpara.slope_speed;
    // else
        powerout = setpara.CamAveSpeed;
    return powerout;
}

int Motor_PID_CalL(int input_speed, int setspeed)
{
    static int powerout, error, last_error, last_last_error;
    error = setspeed - input_speed;
    int d_error = error - last_error;
    int dd_error = -2 * last_error + error + last_last_error;
    // powerout += setpara.stop_kp * d_error + 3 * 0 * error + setpara.stop_kd * dd_error;
    powerout += setpara.stop_kp * d_error + setpara.stop_kd * dd_error;
    last_last_error = last_error;
    last_error = error;

    return powerout;
}

int Motor_PID_CalR(int input_speed, int setspeed)
{
    static int powerout, error, last_error, last_last_error;
    error = setspeed - input_speed;
    int d_error = error - last_error;
    int dd_error = -2 * last_error + error + last_last_error;
    // powerout += setpara.stop_kp * d_error + 3 * 0 * error + setpara.stop_kd * dd_error;
    powerout += setpara.stop_kp * d_error + setpara.stop_kd * dd_error;
    last_last_error = last_error;
    last_error = error;

    return powerout;
}
