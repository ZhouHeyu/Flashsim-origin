#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>


inline double randfrac() { return drand48(); }

double pareto(double a, double b)
{
	double y1;
	double y2;
	double y3;

	y1 = 1 - randfrac();
	y2 = -(1/b)*(log(y1));
	y3 = pow(a, exp(y2));
	return y3;

}

int main(){

	//pareto distribution 
	double val;
	int i;

	for(i = 0; i<1000; i++)
	{
		val = pareto(10.0, 2.0);
		printf("%lf ", val);
	}
	printf("\n");
}


