//kalman test
float in = 0.0;

float x = 0.0;
float P = 1.0;

float var_r = 10.135;
float var_p = 1.0e-2;
float K = 0.0;

// average test
long count = 0;
float mean = 0;
float mean2 = 0;

void setup()
{
  Serial.begin(115200);
}

void loop()
{
  in = analogRead(A2);
  count++;

  mean = (mean*(count-1) + in)/count ;
  
  P = P + var_p;
  K = P/(P+var_r);
  x = x + K*( in-x );
  P = ( 1-K )*P;
  
  Serial.print(in);
  Serial.print(" ");            
  Serial.print(x);
  Serial.print(" ");
  Serial.println(mean);
  delay(20);
}
