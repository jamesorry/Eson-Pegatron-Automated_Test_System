#include "Qformat.h"
#include <avr/wdt.h>
#include "SoftwareSerial.h"
#include "UserCommand.h"
#include "EEPROM_Function.h"
#include "MainProcess.h"
/*
Q表示    S表示    十進制數表示範圍
Q15    S0.15    -1≤x≤0.9999695
Q14    S1.14    -2≤x≤1.9999390
Q13    S2.13    -4≤x≤3.9998779
Q12    S3.12    -8≤x≤7.9997559
Q11    S4.11    -16≤x≤15.9995117
Q10    S5.10    -32≤x≤31.9990234
Q9    S6.9    -64≤x≤63.9980469
Q8    S7.8    -128≤x≤127.9960938
Q7    S8.7    -256≤x≤255.9921875
Q6    S9.6    -512≤x≤511.9804375
Q5    S10.5    -1024≤x≤1023.96875
Q4    S11.4    -2048≤x≤2047.9375
Q3    S12.3    -4096≤x≤4095.875
Q2    S13.2    -8192≤x≤8191.75
Q1    S14.1    -16384≤x≤16383.5
Q0    S15.0    -32768≤x≤32767
*/

#define Q14(X) ((X < 0.0) ? (int)(16384*(X) - 0.5) : (int)(16384*(X) + 0.5))
#define Q0(x) ( (x)<0 ? ((int) (1.0*(x) - 0.5)) : (min(32767,(int) ((32767.0/32768.0)*(x) + 0.5))))
#define Q1(x) ( (x)<0 ? ((int) (2.0*(x) - 0.5)) : (min(32767,(int) ((32767.0/16384.0)*(x) + 0.5))))
#define Q2(x) ( (x)<0 ? ((int) (4.0*(x) - 0.5)) : (min(32767,(int) ((32767.0/8192.0)*(x) + 0.5))))
#define Q13(x) ( (x)<0 ? ((int) (8192.0*(x) - 0.5)) : (min(32767,(int) ((32767.0/4.0)*(x) + 0.5))))
// #define Q14(x) ( (x)<0 ? ((int) (16384.0*(x) - 0.5)) : (min(32767,(int) ((32767.0/2.0)*(x) + 0.5))))
#define Q15(x) ( ((x)<(0.0)) ? ((int) (32768.0*(x) - 0.5)) : (min(32767,(int) (32767.0*(x) + 0.5))))

#define min(a,b)  (((a) < (b)) ? (a) : (b))

double power(double base, int n)
{
    int i;
    double pow = 1.0;

    for (i = 1; i <= n; i++) {
        pow = pow * base;
    }

    return pow;
}

double QFormatIntegerToFloat(const double x,
                             const int n)
{
    return (((double)x) / ((double)(1 << n)));
}

short FloatToQFormatInteger(const double x,
                            const int m,
                            const int n)
{
    return (((x) < (0.0)) ? ((int)(power(2, n) * (x) - 0.5)) : (min(32767, (int)((32767.0 / (power(2, m))) * (x) + 0.5))));
}

void test()
{
    char buff[100];

    // Q14    S1.14    -2≤x≤1.9999390
    short s = FloatToQFormatInteger(0.321, 0, 15);
    short X = FloatToQFormatInteger(0.123, 0, 15);
    Serial.println(s, HEX);
    Serial.println(X, HEX);
    Serial.println(QFormatIntegerToFloat(s+X , 15), 5);

#if 0
    // Q9
    s = FloatToQFormatInteger(-64, 6, 9);
    sprintf("%x, %f\n", s, QFormatIntegerToFloat(s, 9));

    s = FloatToQFormatInteger(64, 6, 9);
    sprintf("%x, %f\n", s, QFormatIntegerToFloat(s, 9));

    // Q15    S0.15    -1≤x≤0.9999695
    s = FloatToQFormatInteger(-1.0, 0, 15);
    sprintf("%x, %f\n", s, QFormatIntegerToFloat(s, 15));

    s = FloatToQFormatInteger(1.0, 0, 15);
    sprintf("%x, %f\n", s, QFormatIntegerToFloat(s, 15));


    // Q0    S15.0    -32768≤x≤32767
    s = FloatToQFormatInteger(-32768, 15, 0);
    sprintf("%x, %f\n", s, QFormatIntegerToFloat(s, 0));

    s = FloatToQFormatInteger(32767, 15, 0);
    sprintf("%x, %f\n", s, QFormatIntegerToFloat(s, 0));
#endif
}