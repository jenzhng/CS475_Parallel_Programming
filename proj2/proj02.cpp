
#include <math.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <omp.h>

#ifndef NUMT
#define NUMT		    4
#endif

#ifndef NUMTRIES
	#define NUMTRIES	100
#endif

#define CSV

const float GRAIN_GROWS_PER_MONTH =	       12.0;
const float ONE_DEER_EATS_PER_MONTH =		1.0;

const float AVG_PRECIP_PER_MONTH =		7.0;	// average
const float AMP_PRECIP_PER_MONTH =		6.0;	// plus or minus
const float RANDOM_PRECIP =			2.0;	// plus or minus noise

const float AVG_TEMP =				60.0;	// average
const float AMP_TEMP =				20.0;	// plus or minus
const float RANDOM_TEMP =			10.0;	// plus or minus noise

const float MIDTEMP =				40.0;
const float MIDPRECIP =				10.0;


// Use variables like NextNumDeer, NextNumGrain in each function.
// State Global Variables 
int	    NowYear;		// 2024 - 2029
int	    NowMonth;		// 0 - 11
int     NowPrintMonth;
float	NowPrecip;		// inches of rain per month
float	NowTemp;		// temperature this month
float	NowHeight;		// grain height in inches
int	    NowNumDeer;		// number of deer in the current population

float NowDeerHeatMod;    // Represents percent of population killed
float NowGrainHeatMod;   // Represents percent of height in grains killed

unsigned int seed = 0;

// Mutex/Barrier Global Variables
omp_lock_t	Lock;
int		NumInThreadTeam;
int		NumAtBarrier;
int		NumGone;


// Returns square root

float SQR( float x )
{
    return x*x;
}

// Returns random float in range
float Ranf(unsigned int *seedp,  float low, float high) {
    float r = (float) rand_r(seedp);
    return(low + r * (high - low) / (float)RAND_MAX);
}


// Returns random int in range
int Ranf(unsigned int *seedp, int ilow, int ihigh) {
    float low = (float)ilow;
    float high = (float)ihigh + 0.9999f;
    return (int)(Ranf(seedp, low,high));
}

// Sets NowPrecip and NowTemp for next environment state
void setNextEnvironment() {
    float ang = (  30.*(float)NowMonth + 15.  ) * ( M_PI / 180. );
    // Compute NowTemp
    float temp = AVG_TEMP - AMP_TEMP * cos( ang );
    NowTemp = temp + Ranf(&seed,-RANDOM_TEMP, RANDOM_TEMP );
    // Compute NowPrecip
    float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin( ang );
    NowPrecip = precip + Ranf(&seed,-RANDOM_PRECIP, RANDOM_PRECIP );
    if( NowPrecip < 0. ) {
        NowPrecip = 0.;
    }
}



void Deer() {
    while(NowYear < 2030) {
        // compute next Number of Deer
        int nextNumDeer;

        // If too many deer, decrease
        if (float(NowNumDeer) > NowHeight) {
            nextNumDeer = NowNumDeer - 1;
        }
        // If more grain than deer, increase
        else if (float(NowNumDeer) < NowHeight) {
            nextNumDeer = NowNumDeer + 1;
        }
        // Else stay the same
        else {
            nextNumDeer = NowNumDeer;
        }
        // Clamp nextNumDeer
        if (nextNumDeer < 0) {
            nextNumDeer = 0;
        }

        // Modify if Heatwave is Present
        if (NowDeerHeatMod > .0) {
            nextNumDeer = nextNumDeer - int(nextNumDeer * NowDeerHeatMod);
        }
        
        #pragma omp barrier

        NowNumDeer = nextNumDeer;

        #pragma omp barrier

        #pragma omp barrier
    }
}

void Grain() {
    while(NowYear < 2030) {
        float nextHeight;

        float tempFactor = exp(-SQR((NowTemp-MIDTEMP)/10.));
        float precipFactor = exp(-SQR((NowPrecip-MIDPRECIP)/10.));

        // Configure nextHeight
        nextHeight = NowHeight;
        nextHeight += tempFactor*precipFactor*GRAIN_GROWS_PER_MONTH;
        nextHeight -= (float)NowNumDeer*ONE_DEER_EATS_PER_MONTH;

        // clamp nextHeight
        if (nextHeight < 0.) {
            nextHeight = 0.;
        }

        // Modify if Heatwave is Present
        if (NowGrainHeatMod > .0) {
            nextHeight = nextHeight - (nextHeight * NowGrainHeatMod);
        }

        #pragma omp barrier

        NowHeight = nextHeight;
        
        #pragma omp barrier

        #pragma omp barrier
    }
}

void Heat() {
    while(NowYear < 2030) {
        // compute next Number of Natural Predators
        int rand = Ranf(&seed, 2, 10);
        float NextDeerHeatMod = 0.0;
        float NextGrainHeatMod = 0.0;
        if(NowMonth == 7){
            NextDeerHeatMod = 0.2;
            NextGrainHeatMod = 0.1;
 
        }

        #pragma omp barrier
        NowDeerHeatMod = NextDeerHeatMod;
        NowGrainHeatMod = NextGrainHeatMod;

        #pragma omp barrier
        
        #pragma omp barrier
    }
}

void Watcher() {
    while(NowYear < 2030) {
 
        #pragma omp barrier

        #pragma omp barrier

        // Increment Time
        if (NowMonth >= 11) {
            NowMonth = 0;
            NowYear++;
        }
        else {
            NowMonth++;
        }
        NowPrintMonth++;

        // Set Next Environment
        setNextEnvironment();

        // Print Results NowNumDeer, NowDeerHeatMod in Percent, NowHeight, NowGrainHeatMod in Percent, NowTemp in Celsius, NowPrecip
    printf("%d, %d,%d,%lf,%d,%lf,%lf\n", NowPrintMonth, NowNumDeer, int(NowDeerHeatMod * 100), (NowHeight * 2.54),int(NowGrainHeatMod * 100),(5./9.)*(NowTemp-32), (NowPrecip * 2.54));

        #pragma omp barrier
    }
}

int main() {
#ifdef _OPENMP
	#ifndef CSV
		fprintf( stderr, "OpenMP is supported -- version = %d\n", _OPENMP );
	#endif
#else
        fprintf( stderr, "No OpenMP support!\n" );
        return 1;
#endif
    // Setup initial Environment variables
    // starting date and time:
    NowMonth = 0;
    NowPrintMonth = 0;
    NowYear  = 2024;

    // starting state (feel free to change this if you want):
    NowNumDeer = 2;
    NowHeight =  5.;
    NowDeerHeatMod = 0.;
    NowGrainHeatMod = 0.;

    // Set initial environment state
    setNextEnvironment();
#ifdef CSV
    printf("Month,NowNumDeer,NowDeerHeatMod,NowHeight,NowGrainHeatMod,NowTemp,NowPrecip\n");
    printf("%d, %d,%d,%lf,%d,%lf,%lf\n", NowPrintMonth, NowNumDeer, int(NowDeerHeatMod * 100), (NowHeight * 2.54),int(NowGrainHeatMod * 100),(5./9.)*(NowTemp-32), (NowPrecip * 2.54));
#endif
    omp_set_num_threads(4);	// same as # of sections
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            Deer();
        }

        #pragma omp section
        {
            Grain();
        }

        #pragma omp section
        {
            Watcher();
        }

        #pragma omp section
        {
            Heat();
        }
    } // implied barrier -- all functions must return in order
	// to allow any of them to get past here
}