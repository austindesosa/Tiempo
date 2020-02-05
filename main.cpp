#include <math.h>
#include <stdio.h>
#include <ctime>


double PI = M_PI;
double DEG = PI / 180;
double LIGHT = 299458792.0;

int extract_bit(int bitno, unsigned int val)  {
unsigned int xtr_val = (unsigned int) pow(2,bitno);
int ret = 1;
if((xtr_val & val)==0){ret -= 1;};
return ret;
}

unsigned int max_uint(int size){
  return (unsigned int) pow(2,size) - 1;
}

class Entity{
  public:

  int clk_count = 0;
  int SIZE_IN = 5;
  int n_clk = 100;
  unsigned int in_val = 0;
  unsigned int out_val;

  void tick(void){
    //One clock cycle
    clk_count += 1;
  }

  void wait(int n_ticks){
    int clk_next = clk_count + n_ticks;
    while(clk_count < clk_next){
      tick();
    }

  }

  unsigned int arch(int val){
    unsigned int ret = 0x0;
    unsigned int a,b;
    a = extract_bit(0,val);
    b = extract_bit(1,val);
    ret += a ^ b;

    return ret ;
  }

  void sweep(void){
    out_val = arch(in_val);
  }


  void feed(unsigned int val){
    in_val = val;
    sweep();
  }
  
  void report(void){
    printf("clk_count = %i \nin_val = %x \nout_val = %x\n\n",clk_count,in_val,out_val);
  }

  void test_step(){
    sweep();
    report();
    int next_count = clk_count + n_clk;
    while(clk_count < next_count){tick();};
  }

  void testbench(){
    int max_val = (int) pow(2, SIZE_IN) - 1;
    printf("Entity.testbench() invoked\n\n");
    feed(0);
    while(in_val <= max_val) {
      test_step();
      in_val += 1;
    };
    printf("Entity.testbench() complete\n\n\n");
  }
 
};

class Dac{
  //This class represents a digital-analog converter
  public:
  int clk_count = 0;
  unsigned int dig_in = 0x0; //digital input
  double an_out; //analog output
  double MAX_OUT = 5.0; //Maximum analog output
  int SIZE_IN = 12; //Number of input bits
  unsigned int MAX_IN = max_uint(SIZE_IN); //Maximum digital input
  double prec = MAX_OUT / MAX_IN; //Precision, in output volts per input unit

  void report(void){
    //Prints string to console containing
    //clock, input, and output values for this Dac object.
    printf("clk_count = %i \ndig_in = %x \nan_out = %f \n\n",clk_count,dig_in,an_out);
  }

  void tick(void) {
    //Increments clk_couont, representing obn=ne clock cycle or counter tick
    clk_count += 1;
  }


  void wait(int n_ticks) {
    int clk_next = n_ticks + clk_count;
    while(clk_count < clk_next){
      tick();
    }
  }


  void sweep(void){
    MAX_IN = max_uint(SIZE_IN);
    prec = MAX_OUT / MAX_IN;
    an_out = dig_in * prec ; 
  }

  void truncate(unsigned int val) {
    if(val > MAX_IN){
      val = MAX_IN;

    }  else {};
  }

  unsigned int to_dig(double an_val) {
    unsigned int n_units = (unsigned int) (an_val / prec);
    truncate(n_units);
    return( n_units );

  }

  void feed(unsigned int val) {
    truncate(val);
    dig_in = val;
    sweep();

  }

  double sin_dub(int period) {
    double baseline = MAX_OUT / 2;
    double theta = (double) clk_count / (double) period;
    theta *= 2 *  PI;
    double wave = 1 + sin(theta);
    wave *= baseline;
    return wave ;
  }

  void dac_sin(int period) {
    double an = sin_dub(period);
    feed(to_dig(an));

  }



  double decay_dub(int tau){
    //printf("Dac.decay_dub( %i ) invoked\n",tau);
    double t = (double) clk_count / (double) tau;
    //printf("t = %f\n",t);
    double y = exp(-1 * t);
    //printf("y = %f \n",y);
    y *= MAX_OUT;
    //printf("returning y = %f \nDac.decay_dub( %i ) complete\n\n",y, tau);
    return y;
  }

  void dac_decay(int tau){
    //@param   tau   is the Decay constant
    double an = decay_dub(tau);
    unsigned long an_dig = to_dig(an);
    feed(an_dig);
  }

  double pwr_dub(double pwr, double g){
    //@param   pwr   is desired ourput power, in watts
    //@param   g   is electrical load conductance, in mhos
    double vsq = pwr / g;
    if(vsq<0){vsq *= -1;};
    double voltage = pow(vsq, 0.5);
    return voltage ;
  }

  void dac_pwr(double pwr, double g){
    double v = pwr_dub(pwr,g);
    unsigned int v_dig = to_dig(v);
    feed(v_dig);
  }

  

};

int main(void) {
  Dac dax = Dac();
  int pd = 360;
  while(dax.clk_count <= pd) {
    dax.dac_sin(pd);
    dax.report();
    dax.tick();
  }
}