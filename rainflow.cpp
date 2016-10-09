
#include <cstdlib>
#include <iostream>

#include <vector>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>


#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>

#define MAX 51000000

using namespace std;

class Rainflow
{
    private:

        float C[100];
        float AverageMean[100];
        float MaxPeak[100];
        float MinValley[100];

        float MaxAmp[100];
        float AverageAmp[100];

        double L[20];

        double ylast;

        float sum;
        float ymax;

        float mina,maxa;

        float t;
        float scale;
        float X,Y;

        long ijk;

        long kv;

        long hold;
        long i,j,k,n;
        long num;

        long nkv;

        int ic,iscale;

        long N,NP;

        long last_a;

        FILE *pFile[6];
        char filename[6][FILENAME_MAX];

        vector<float> y;
        vector<vector<float> > B;  // need space > >

    public:
        void read_data(void);
        void rainflow_engine(void);
        void damage_index(void);
        void print_data(void);
};


int main()
{
    Rainflow rf;

	printf("\n ");
	printf("\n rainflow.cpp  ver 2.8  June 24, 2014 \n");
	printf("\n by Tom Irvine  Email: tom@vibrationdata.com \n");
	printf("\n ASTM E 1049-85 (2005) Rainflow Counting Method \n");

    time_t start = time(0);

    rf.read_data();
    rf.rainflow_engine();
    rf.print_data();

    int icd;
	printf("\n\n Calculate relative damage index D?  1=yes 2=no \n");
	scanf("%d",&icd);

	if(icd==1)
	{
        rf.damage_index();
	}

    time_t end = time(0);
    double time = difftime(end, start);

    printf("\n Elapsed Time = %8.4g sec \n",time);


	printf("\n Press any key to exit.\n");

	getch();

	exit(1);

}
void Rainflow::print_data()
{
    printf("\n Amplitude = (peak-valley)/2 \n");

    fprintf(pFile[1],"\n Amplitude = (peak-valley)/2 \n");

//*****************************************************************************


	          printf("\n ");
	          printf("\n          Range            Cycle       Ave      Max     Ave     Min       Max");
	          printf("\n         (units)           Counts      Amp      Amp     Mean    Valley    Peak \n");

	for(i=13;i>=1;i--)
    {
    	printf("  %8.2lf to %8.2lf\t%8.1lf\t%6.4g\t%6.4g\t%6.4g\t%6.4g\t %6.4g\n",L[i],L[i+1],C[i],AverageAmp[i],MaxAmp[i],AverageMean[i],MinValley[i],MaxPeak[i]);
    }

	fprintf(pFile[1],"\n ");
	fprintf(pFile[1],"\n          Range            Cycle       Ave     Max     Ave     Min      Max");
	fprintf(pFile[1],"\n         (units)           Counts      Amp     Amp     Mean    Valley   Peak \n");

	for(i=13;i>=1;i--)
    {
    	fprintf(pFile[1],"  %8.4lf to %8.4lf\t%8.1lf\t%6.4g\t%6.4g\t%6.4g\t %6.4g\t %6.4g\n",L[i],L[i+1],C[i],AverageAmp[i],MaxAmp[i],AverageMean[i],MinValley[i],MaxPeak[i]);

     	fprintf(pFile[2]," %ld \t %g \n",i,C[i]);
    }

	fclose(pFile[0]);
	fclose(pFile[1]);
	fclose(pFile[2]);
	fclose(pFile[3]);
	fclose(pFile[4]);

	printf("\n\n  Total Cycles = %g  hold=%ld  NP=%ld ymax=%g\n",sum,hold,NP,ymax);
	fprintf(pFile[1],"\n\n  Total Cycles = %g  hold=%ld  NP=%ld ymax=%g\n",sum,hold,NP,ymax);

    printf("\n\n The output files are: \n");

    printf(" %s \n",filename[1]);
	printf(" %s \n",filename[2]);
	printf(" %s \n",filename[3]);
	printf(" %s \n",filename[4]);

}
void Rainflow::read_data(void)
{
    float aa;

	printf("\n The input file must be a time history. \n");
	printf("\n Select format: ");
	printf("\n   1=amplitude ");
	printf("\n   2=time & amplitude \n");

	scanf("%d",&ic);

	if(ic==1)
	{
   		printf( "\n\n The base input file must contain one column: unit \n");
	}
	else
	{
   		printf( "\n\n The base input file must contain two columns: \n");
		printf( " time & unit    \n");
	}

	printf( "\n Input filename \n");
	scanf("%s",filename[0]);

    pFile[0] = fopen(filename[0], "rb");

	while(pFile[0] == NULL )
	{

		printf("\n Failed to open file: %s \n", filename[0]);

		printf("\n Please enter the input filename: \n");

		scanf("%s",filename[0]);

		pFile[0] = fopen(filename[0], "rb");

	}
	printf("\n File: %s opened. \n", filename[0]);

	printf( "\n\n Enter the output table filename: \n");
	scanf("%s",filename[1]);

	pFile[1]=fopen(filename[1], "w");

    strcpy(filename[2],"rainflow_graph.out");
	pFile[2]=fopen(filename[2], "w");

    strcpy(filename[3],"range_cycles.out");
	pFile[3]=fopen(filename[3], "w");

    strcpy(filename[4],"amp_cycles.out");
	pFile[4]=fopen(filename[4], "w");


//    strcpy(filename[5],"points.out");
//	pFile[5]=fopen(filename[5], "w");


	i=0;

	if(ic==1)
	{
		while( fscanf(pFile[0],"%f",&aa)>0)
		{
		    y.push_back(aa);

			i++;

			if(i==MAX)
			{
			    printf("\n Warning:  input data limit reached \n.");
				break;
			}
		}
	}
	else
	{
		while( fscanf(pFile[0],"%f %f",&t,&aa)>0)
		{
		    y.push_back(aa);

			i++;

			if(i==MAX)
			{
			    printf("\n Warning:  input data limit reached \n.");
				break;
			}
		}
	}
	ylast=y[i-1];

	NP=i+1;


//	printf("\n ref 1: last_a = %ld \n",last_a);

	printf("\n ");
	printf("\n Multiply data by scale factor?");
	printf("\n 1=yes  2=no \n");
	scanf("%d",&iscale);

	if(iscale==1)
	{
    	printf("\n Enter scale factor \n");
    	scanf("%f",&scale);

		for(i=0;i<NP;i++)
		{
			y[i]*=scale;
		}
	}
}
void Rainflow::damage_index(void)
{
    double b;
    double D=0;
    double Y;

    printf("\n\n Enter fatigue exponent: ");
    scanf("%lf",&b);

    for(long i=0;i<=kv;i++)
    {
        Y=B[i][0];
        D+=B[i][1]*pow((Y/2.),b);
    }
    printf("\n D=%8.4g \n",D);

}
void Rainflow::rainflow_engine(void)
{

    vector<float> a;

    double slope1;
    double slope2;

    ymax=0.;

    nkv=0;

	k=0;
//	a[k]=y[k];
    a.push_back(y[k]);

	k=1;


    for(i=1;i<(NP-1);i++)
	{

        slope1=(  y[i]-y[i-1]);
        slope2=(y[i+1]-y[i]);

        if((slope1*slope2)<=0. && fabs(slope1)>0.)
        {
          a.push_back(y[i]);
          k++;
        }

	}
    a.push_back(ylast);
    k++;

	last_a=k-1;

    hold=last_a;

    long WIDTH=4;
    long HEIGHT=0;
    B.resize(HEIGHT);

    for(i=0; i < HEIGHT; i++)
    {
        B[i].resize(WIDTH);
    }

 //   printf(" H=%ld W=%ld ",HEIGHT,WIDTH);
 //   getch();


	mina=100000;
	maxa=-mina;

	for(i=0;i<=last_a;i++)
	{
		if(a[i]<mina)
		{
			mina=a[i];
		}
		if(a[i]>maxa)
		{
			maxa=a[i];
		}

//		fprintf(pFile[5]," %8.4g \n",a[i]);
	}

	num=long(maxa-mina)+1;

	n=0;
	i=0;
	j=1;

	sum=0;

	kv=0;

	long LLL=last_a;

	std::vector<float> row(4);

    printf("\n percent completed \n");

	while(1)
	{
    	Y=(fabs(a[i]-a[i+1]));
    	X=(fabs(a[j]-a[j+1]));

    	if(X>=Y && Y>0 && Y<1.0e+20)
		{
            if(Y>ymax)
            {ymax=Y;}

        	if(i==0)
			{

           		n=0;
           		sum+=0.5;

                row[3]=a[i+1];
                row[2]=a[i];
                row[1]=0.5;
                row[0]=Y;

                B.push_back(row);

//                printf("1 %8.4g %8.4g %8.4g %8.4g \n",B[kv][0],B[kv][1],B[kv][2],B[kv][3]);

                kv++;

				a.erase (a.begin());

				last_a--;

           		i=0;
           		j=1;
			}
        	else
			{
           		sum+=1;

                row[3]=a[i+1];
                row[2]=a[i];
                row[1]=1.;
                row[0]=Y;

                B.push_back(row);
//                printf("2 %8.4g %8.4g %8.4g %8.4g \n",B[kv][0],B[kv][1],B[kv][2],B[kv][3]);

                kv++;


           		n=0;

                a.erase (a.begin()+(i+1));
                a.erase (a.begin()+i);


				last_a-=2;

           		i=0;
           		j=1;
			}

			nkv++;

			if(nkv==3000)
			{
               double ratio =  fabs((last_a)/double(LLL));

			   printf(" %3.1lf \n",(1-ratio)*100.);
			   nkv=0;
           }
		}
    	else
		{
        	i++;
        	j++;
		}

    	if((j+1)>last_a)
		{
        	break;
		}

	}


	for(i=0;i<(last_a);i++)
	{
    	Y=(fabs(a[i]-a[i+1]));
    	if(Y>0. && Y<1.0e+20)
		{
        	sum+=0.5;

            row[3]=a[i+1];
            row[2]=a[i];
            row[1]=0.5;
            row[0]=Y;
            B.push_back(row);
//            printf("3  %8.4g %8.4g %8.4g %8.4g \n",B[kv][0],B[kv][1],B[kv][2],B[kv][3]);

            kv++;

            if(Y>ymax)
            {ymax=Y;}

		}

	}


    L[1]=0;
    L[2]=2.5;
    L[3]=5;
    L[4]=10;
    L[5]=15;
    L[6]=20;
    L[7]=30;
    L[8]=40;
    L[9]=50;
    L[10]=60;
    L[11]=70;
    L[12]=80;
    L[13]=90;
    L[14]=100;

    for(ijk=1;ijk<=14;ijk++)
    {
        L[ijk]*=ymax/100.;

        C[ijk]=0.;
        AverageMean[ijk]=0.;
        MaxPeak[ijk]=-1.0e+20;
        MinValley[ijk]= 1.0e+20;

        MaxAmp[ijk]=-1.0e+20;
        AverageAmp[ijk]= 1.0e+20;

 //       printf("  L[%ld]=%g ymax=%8.4g\n",ijk,L[ijk],ymax);
    }

 //   getch();

    kv--;

    for(ijk=13;ijk>=0;ijk--)
    {
        AverageAmp[ijk]=0.;
    }


    for(i=0;i<=kv;i++)
    {
        Y=B[i][0];

//        printf(" %ld %ld %10.4e \t %3.1f \n",i,kv,Y,B[i][1]);

        fprintf(pFile[3]," %10.4e \t %3.1f \n",Y,B[i][1]);
        fprintf(pFile[4]," %10.4e \t %3.1f \n",Y/2.,B[i][1]);

//     printf("i=%d Y=%g \n",i,Y);

        for(ijk=13;ijk>=0;ijk--)
        {
//           printf(" %8.4g  %8.4g   %8.4g \n",Y,L[ijk],L[ijk+1]);

            if(Y>=L[ijk] && Y<=L[ijk+1])
            {
                C[ijk]=C[ijk]+B[i][1];
                AverageMean[ijk]+=B[i][1]*(B[i][3]+B[i][2])*0.5; // weighted average

                if(B[i][3]>MaxPeak[ijk])
                {
                    MaxPeak[ijk]=B[i][3];
                }
                if(B[i][2]>MaxPeak[ijk])
                {
                    MaxPeak[ijk]=B[i][2];
                }

                if(B[i][3]<MinValley[ijk])
                {
                    MinValley[ijk]=B[i][3];
                }
                if(B[i][2]<MinValley[ijk])
                {
                    MinValley[ijk]=B[i][2];
                }

                if(Y>MaxAmp[ijk])
                {
                    MaxAmp[ijk]=Y;
                }

                AverageAmp[ijk]+=B[i][1]*Y*0.5;

                break;
            }
        }
    }

    for(ijk=1;ijk<=14;ijk++)
    {
        if(C[ijk]>0)
        {
            AverageMean[ijk]/=C[ijk];
            AverageAmp[ijk]/=C[ijk];
        }
        MaxAmp[ijk]/=2.;

        if(C[ijk]<0.5)
        {
            AverageAmp[ijk]=0.;
            MaxAmp[ijk]=0.;
            AverageMean[ijk]=0.;
            MinValley[ijk]=0.;
            MaxPeak[ijk]=0.;
        }
//        printf(" %8.4g  %8.4g  %8.4g  %8.4g  %8.4g \n",AverageAmp[ijk],MaxAmp[ijk],AverageMean[ijk],MinValley[ijk],MaxPeak[ijk]);
    }
}
