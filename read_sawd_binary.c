#include <stdio.h>
#include <stdlib.h>
#include <vicNl.h>
 
void read_sawd_binary(atmos_data_struct *temp,
		      FILE              *sawdf,
		      int               *nrecs,
		      int                dt,
		      int                prec)
/**********************************************************************
  read_sawd_binary	Keith Cherkauer		April 26, 1998

  This routine reads in atmospheric data values from surface
  airways binary gridded hourly data files.

				Input	        Output
				Units	        Units
	air_temp	-	C*100	        C
	pressure	-	Pa*100	        kPa
	wind		-	m/s*1000        m/s
	rel_humid	-	%*100		%
	tskc		-	%*10	        fract

**********************************************************************/
{
  extern debug_struct debug;
  extern param_set_struct param_set;
 
  int       i, n, rec;
  int       fixcnt;
  char      str[210];
  char      tmpmem[20];
  short int values[5];

  /** Count Records **/
  n = 0;
  fread(tmpmem,sizeof(char),10,sawdf);
  while (!feof(sawdf)) {
    n++;
    fread(tmpmem,sizeof(char),10,sawdf);
  }
  printf("nrecs = %d\n",n);
  if(*nrecs*dt>n) {
    fprintf(stderr,"WARNING: SAWD file does not have as many records as defined in the global parameter file, truncating run to %i records.\n",n);
    *nrecs=n/dt;
  }
  if(*nrecs*dt<n) {
    fprintf(stderr,"WARNING: SAWD file has more records then were defined in the global parameter file, run will stop after %i records.\n",*nrecs*dt);
    n = *nrecs*dt;
  }

  rewind(sawdf);

  /** Check for Header, and Skip **/
  fixcnt = 0;
  rec = 0;
  for (i = 0; i < n; i++) {
    if(i == rec*dt) {
      fread(values,sizeof(short int),5,sawdf);
      temp[rec].air_temp = (double)values[0]/100.;
      temp[rec].pressure = (double)values[1]/100.;
      if(temp[rec].pressure == 0. && rec>0) {
	temp[rec].pressure = temp[rec-1].pressure;
	fixcnt++;
      }
      temp[rec].wind = (double)values[2]/1000.;
      temp[rec].rel_humid = (double)values[3]/100.;
      if(temp[rec].rel_humid == 0. && rec>0) {
	temp[rec].rel_humid = temp[rec-1].rel_humid;
	fixcnt++;
      }
      if(temp[rec].rel_humid > 100.) {
	temp[rec].rel_humid = 100.;
	fixcnt++;
      }
      temp[rec].tskc = (double)values[4] / 1000.;
      if(prec) {
	fscanf(sawdf,"%s",str);
	temp[rec].prec = atof(str);
      }
      rec++;
    }
    else fread(values,sizeof(short int),5,sawdf);

  }

  if(fixcnt>0) {
    fprintf(stderr,"WARNING: Had to fix %i values in sawd file.\n",fixcnt);
  }

  param_set.WIND = param_set.AIR_TEMP = param_set.PRESSURE = TRUE;
  param_set.TSKC = param_set.REL_HUMID = TRUE;
  if(prec) param_set.PREC = TRUE;

}
