#line 156 "c.m4.null"

#line 1 "water.C"
/********************************************************************************
	ESLBench Benchmark Suite
	Authors: Liana Duenha
	Supervisor: Rodolfo Azevedo
	Date: July-2012
	

	Computer Systems Laboratory (LSC)
	IC-UNICAMP
	http://www.lsc.ic.unicamp.br/
	
	
	This source code is part of the ESLBench Benchmark Suite, which is a free
	source-code benchmark for evaluation of Electronic Systemc Level designs.
	This benchmakr is distributed int he hope that it will be useful, but 
	without any warranty.
 
*********************************************************************************/


/*************************************************************************/
/*                                                                       */
/*  Copyright (c) 1994 Stanford University                               */
/*                                                                       */
/*  All rights reserved.                                                 */
/*                                                                       */
/*  Permission is given to use, copy, and modify this software for any   */
/*  non-commercial purpose as long as this copyright notice is not       */
/*  removed.  All other uses, including redistribution in whole or in    */
/*  part, are forbidden without prior written permission.                */
/*                                                                       */
/*  This software is provided with absolutely no warranty and no         */
/*  support.                                                             */
/*                                                                       */
/*************************************************************************/


#line 37
	#include <sys/time.h>
#line 37
	#include <unistd.h>
#line 37
	#include <stdlib.h>
#line 37
	#include <malloc.h>	
#line 37
	#define MAX_THREADS 32	
#line 37
			
#line 37
 

/*  Usage:   water < infile,
    where infile has 10 fields which can be described in order as
    follows:

    TSTEP:   the physical time interval (in sec) between timesteps.
    Good default is 1e-15.
    NMOL:    the number of molecules to be simulated.
    NSTEP:   the number of timesteps to be simulated.
    NORDER:  the order of the predictor-corrector method to be used.
    set this to 6.
    NSAVE:   the frequency with which to save data in data collection.
    Set to 0 always.
    NRST:    the frequency with which to write RST file: set to 0 always (not used).
    NPRINT:  the frequency with which to compute potential energy.
    i.e. the routine POTENG is called every NPRINT timesteps.
    It also computes intermolecular as well as intramolecular
    interactions, and hence is very expensive.
    NFMC:    Not used (historical artifact).  Set to anything, say 0.
    NumProcs: the number of processors to be used.
    CUTOFF:  the cutoff radius to be used (in Angstrom,
    floating-point).  In a real simulation, this
    will be set to 0 here in which case the program will
    compute it itself (and set it to about 11 Angstrom.
    It can be set by the user if they want
    to use an artificially small cutoff radius, for example
    to control the number of boxes created for small problems
    (and not have fewer boxes than processors).
    */

#include <stdio.h>
#include <string.h>
#include <math.h>

/*  include files for declarations  */
#include "cnst.h"
#include "fileio.h"
#include "frcnst.h"
#include "mdvar.h"
#include "parameters.h"
#include "randno.h"
#include "split.h"
#include "water.h"
#include "wwpot.h"
#include "mddata.h"
#include "global.h"

#define NUM_PROC 1

long NATOMS;
long I2;
long NMOL,NORDER,NATMO,NATMO3,NMOL1;
long BOX_PER_SIDE, BPS_SQRD;
long IX[3*MXOD2+1], IRST,NVAR,NXYZ,NXV,IXF,IYF,IZF,IMY,IMZ;
long NumBoxes;

double UNITT,UNITL,UNITM,BOLTZ,AVGNO,PCC[11];
double FC11,FC12,FC13,FC33,FC111,FC333,FC112,FC113,FC123,FC133,FC1111,FC3333,FC1112,FC1122,FC1113,FC1123,FC1133,FC1233,FC1333;
double TLC[100], FPOT, FKIN;
double TEMP,RHO,TSTEP,BOXL,BOXH,CUTOFF,CUT2;
double BOX_LENGTH;
double R3[128],R1;
double OMAS,HMAS,WTMOL,ROH,ANGLE,FHM,FOM,ROHI,ROHI2;
double QQ,A1,B1,A2,B2,A3,B3,A4,B4,AB1,AB2,AB3,AB4,C1,C2,QQ2,QQ4,REF1,REF2,REF4;

FILE *six;

box_type ***BOX;

box_list **my_boxes;

struct GlobalMemory *gl;        /* pointer to the Global Memory
                                   structure, which contains the lock,
                                   barrier, and some scalar variables */

long NSTEP, NSAVE, NRST, NPRINT,NFMC;
long NORD1;
long II;                         /*  variables explained in common.h */
long i;
long NDATA;
long NFRST=11;
long NFSV=10;
long LKT=0;

first_last_array **start_end; /* ptr to array of start/end box #s */
long NumProcs;                 /* number of processors being used;
                                 run-time input           */

double XTT;

unsigned volatile int procCounter = 0;

int main0(int argc, char *argv[]);

int main(int argc, char *argv[])
{
  register int procNumber;

  NumProcs = NPROC;

  AGlobalLock();
      procNumber = procCounter++;
  RGlobalLock();
  
  if (procNumber == 0)
  {	
		printf("\n");
		printf("\n");
        	printf("--------------------------------------------------------------------\n");
		printf("--------------------------  MPSoCBench  ----------------------------\n");
        	printf("-------------------- Running: water-spatial ------------------------\n");
        	printf("--------------- The results will be available in -------------------\n");
	      	printf("------------------------- the output file --------------------------\n");
        	printf("--------------------------------------------------------------------\n");
		printf("\n");
		
		/* OUTPUT FILE */
		six = fopen("output","w");
		if (six == NULL){
			    printf("Error: fopen()\n");
			    exit(1);
		}

	        main0(argc, argv);
		fclose(six);
  }
  else{
      WorkStart();
  }
  {
#line 167
	_exit(0); // To avoid cross-compiler exit routine
#line 167
  	return 0; // Never executed, just for compatibility
#line 167
};	

}
//=============================================

int main0(int argc, char **argv)
{
    /* default values for the control parameters of the driver */
    /* are in parameters.h */

    if ((argc == 2) && ((strncmp(argv[1],"-h",strlen("-h")) == 0) || (strncmp(argv[1],"-H",strlen("-H")) == 0))) {
        printf("Usage:  WATER-SPATIAL < infile, where the contents of infile can be\nobtained from the comments at the top of water.C and the first scanf \nin main() in water.C\n\n");
        exit(0);
    }

        /*  POSSIBLE ENHANCEMENT:  One might bind the first process to a processor
            here, even before the other (child) processes are bound later in mdmain().
            */

    //six = stdout;

    TEMP  =298.0;
    RHO   =0.9980;
    
    /* read input */

    TSTEP    = 1e-15;
    NMOL     =  64; //8; //64; //216; //125;	//Number of Molecules must be an integer cube (8, 27, 64, 343, 512 ...)
    NSTEP    = 1;  // originalmente 3
    NORDER   = 6;
    NSAVE    = 0; //-1
    NRST     = 0; //3000;
    NPRINT   = 3;
    NFMC     = 0;
    //NumProcs = NUM_PROC;	//Number of Processors
    CUTOFF   = 6.212752;

    printf("Using %ld procs on %ld steps of %ld mols\n", NumProcs, NSTEP, NMOL);
    printf("Other parameters:\n\tTSTEP = %8.2e\n\tNORDER = %ld\n\tNSAVE = %ld\n",TSTEP,NORDER,NSAVE);
    printf("\tNRST = %ld\n\tNPRINT = %ld\n\tNFMC = %ld\n\tCUTOFF = %lf\n\n",NRST,NPRINT,NFMC,CUTOFF);

    fprintf(six,"Using %ld procs on %ld steps of %ld mols\n", NumProcs, NSTEP, NMOL);
    fprintf(six,"Other parameters:\n\tTSTEP = %8.2e\n\tNORDER = %ld\n\tNSAVE = %ld\n",TSTEP,NORDER,NSAVE);
    fprintf(six,"\tNRST = %ld\n\tNPRINT = %ld\n\tNFMC = %ld\n\tCUTOFF = %lf\n\n",NRST,NPRINT,NFMC,CUTOFF);


    /* set up scaling factors and constants */

    NORD1=NORDER+1;

    CNSTNT(NORD1,TLC);  /* sub. call to set up constants */

    SYSCNS();    /* sub. call to initialize system constants  */

    printf("%ld boxes with %ld processors\n\n",
           BOX_PER_SIDE * BOX_PER_SIDE * BOX_PER_SIDE, NumProcs);

    if (NumProcs > (BOX_PER_SIDE * BOX_PER_SIDE * BOX_PER_SIDE)) {
        fprintf(stderr,"ERROR: less boxes (%ld) than processors (%ld)\n",
                BOX_PER_SIDE * BOX_PER_SIDE * BOX_PER_SIDE, NumProcs);
        fflush(stderr);
        exit(-1);
    }

    fprintf(six,"\nTEMPERATURE                = %8.2f K\n",TEMP);
    fprintf(six,"DENSITY                    = %8.5f G/C.C.\n",RHO);
    fprintf(six,"NUMBER OF MOLECULES        = %8ld\n",NMOL);
    fprintf(six,"NUMBER OF PROCESSORS       = %8ld\n",NumProcs);
    fprintf(six,"TIME STEP                  = %8.2e SEC\n",TSTEP);
    fprintf(six,"ORDER USED TO SOLVE F=MA   = %8ld \n",NORDER);
    fprintf(six,"NO. OF TIME STEPS          = %8ld \n",NSTEP);
    fprintf(six,"FREQUENCY OF DATA SAVING   = %8ld \n",NSAVE);
    fprintf(six,"FREQUENCY TO WRITE RST FILE= %8ld \n",NRST);
    fflush(six);

    { /* do memory initializations */

        long procnum, i, j, k, l;
        struct list_of_boxes *temp_box;
        long xprocs, yprocs, zprocs;
        long x_inc, y_inc, z_inc;
        long x_ct, y_ct, z_ct;
        long x_left, y_left, z_left;
        long x_first, y_first, z_first;
        long x_last, y_last, z_last;
        double proccbrt;
        long gmem_size = sizeof(struct GlobalMemory);

        {;};  /* macro call to initialize
                                      shared memory etc. */


        /* Allocate space for main (BOX) data structure as well as
         * synchronization variables
         */

        start_end = (first_last_array **)
            our_malloc(sizeof(first_last_array *) * NumProcs);;
        for (i=0; i < NumProcs; i++) {
            start_end[i] = (first_last_array *)
                our_malloc(sizeof(first_last_array));;
        }

        /* Calculate start and finish box numbers for processors */

        xprocs = 0;
        yprocs = 0;
        proccbrt = (double) pow((double) NumProcs, 1.0/3.0) + 0.00000000000001;
        j = (long) proccbrt;
        if (j<1) j = 1;
        while ((xprocs == 0) && (j>0)) {
            k = (long) sqrt((double) (NumProcs / j));
            if (k<1) k=1;
            while ((yprocs == 0) && (k>0)) {
                l = NumProcs/(j*k);
                if ((j*k*l) == NumProcs) {
                    xprocs = j;
                    yprocs = k;
                    zprocs = l;
                } /* if */
                k--;
            } /* while yprocs && k */
            j--;
        } /* while xprocs && j */

        fprintf(six,"xprocs = %ld\typrocs = %ld\tzprocs = %ld\n",
               xprocs, yprocs, zprocs);
        fflush(six);

        /* Fill in start_end array values */

        procnum = 0;
        x_inc = BOX_PER_SIDE/xprocs;
        y_inc = BOX_PER_SIDE/yprocs;
        z_inc = BOX_PER_SIDE/zprocs;

        x_left = BOX_PER_SIDE - (xprocs*x_inc);
        y_left = BOX_PER_SIDE - (yprocs*y_inc);
        z_left = BOX_PER_SIDE - (zprocs*z_inc);
        fprintf(six,"x_inc = %ld\t y_inc = %ld\t z_inc = %ld\n",x_inc,y_inc,z_inc);
        fprintf(six,"x_left = %ld\t y_left = %ld\t z_left = %ld\n",x_left,y_left,z_left);
        fflush(stdout);


        x_first = 0;
        x_ct = x_left;
        x_last = -1;
        x_inc++;
        for (i=0; i<xprocs; i++) {
            y_ct = y_left;
            if (x_ct == 0) x_inc--;
            x_last += x_inc;
            y_first = 0;
            y_last = -1;
            y_inc++;
            for (j=0; j<yprocs; j++) {
                z_ct = z_left;
                if (y_ct == 0) y_inc--;
                y_last += y_inc;
                z_first = 0;
                z_last = -1;
                z_inc++;
                for (k=0; k<zprocs; k++) {
                    if (z_ct == 0) z_inc--;
                    z_last += z_inc;
                    start_end[procnum]->box[XDIR][FIRST] = x_first;
                    start_end[procnum]->box[XDIR][LAST] =
                        min(x_last, BOX_PER_SIDE - 1);
                    start_end[procnum]->box[YDIR][FIRST] = y_first;
                    start_end[procnum]->box[YDIR][LAST] =
                        min(y_last, BOX_PER_SIDE - 1);
                    start_end[procnum]->box[ZDIR][FIRST] = z_first;
                    start_end[procnum]->box[ZDIR][LAST] =
                        min(z_last, BOX_PER_SIDE - 1);
                    z_first = z_last + 1;
                    z_ct--;
                    procnum++;
                }
                y_first = y_last + 1;
                y_ct--;
            }
            x_first = x_last + 1;
            x_ct--;
        }

        /* Allocate space for my_boxes array */

        my_boxes = (box_list **) our_malloc(NumProcs * sizeof(box_list *));;

        /* Set all box ptrs to null */

        for (i=0; i<NumProcs; i++) my_boxes[i] = NULL;

        /* Set up links for all boxes for initial interf and intraf */

        temp_box = my_boxes[0];
        while (temp_box) {
            temp_box = temp_box->next_box;
        }

        /* Allocate space for BOX array */

        BOX = (box_type ***) our_malloc(BOX_PER_SIDE * sizeof(box_type **));;
        for (i=0; i < BOX_PER_SIDE; i++) {
            BOX[i] = (box_type **) our_malloc(BOX_PER_SIDE * sizeof(box_type *));;
            for (j=0; j < BOX_PER_SIDE; j++) {
                BOX[i][j] = (box_type *) our_malloc(BOX_PER_SIDE * sizeof(box_type));;
                for (k=0; k < BOX_PER_SIDE; k++) {
                    BOX[i][j][k].list = NULL;
                    {mutex_init (&(BOX[i][j][k].boxlock)); };
                }
            }
        } /* for i */

        gl = (struct GlobalMemory *) our_malloc(gmem_size);;

        /* macro calls to initialize synch variables  */

        {		
#line 385
	barrier_init (&(gl->start), NumProcs);
#line 385
};
        {		
#line 386
	barrier_init (&(gl->InterfBar), NumProcs);
#line 386
};
        {		
#line 387
	barrier_init (&(gl->PotengBar), NumProcs);
#line 387
};
        {mutex_init (&(gl->IOLock)); };
        {mutex_init (&(gl->IndexLock)); };
        {mutex_init (&(gl->IntrafVirLock)); };
        {mutex_init (&(gl->InterfVirLock)); };
        {mutex_init (&(gl->KinetiSumLock)); };
        {mutex_init (&(gl->PotengSumLock)); };
	join_init(&myJoinPoint, NumProcs);
    }

    fprintf(six,"SPHERICAL CUTOFF RADIUS    = %8.4f ANGSTROM\n",CUTOFF);
    fflush(six);

    IRST=0;

    /* call initialization routine */

    INITIA();

    gl->tracktime = 0;
    gl->intratime = 0;
    gl->intertime = 0;

    /* initialize Index to 1 so that the first created child gets
       id 1, not 0 */

    gl->Index = 1;

    if (NSAVE > 0) {  /* not true for input decks provided */
        fprintf(six,"COLLECTING X AND V DATA AT EVERY %4ld TIME STEPS \n",NSAVE);
    }

    /* spawn helper processes */
    {	
#line 420
	long time(); (gl->computestart) = time(0);
#line 420
};
    
    printf("...\n");
    WorkStart();
    printf("...\n");
    /* macro to make main process wait for all others to finish */
    {
#line 426
	wait_for_end(&(myJoinPoint));
#line 426
};
    {	
#line 427
	long time(); (gl->computeend) = time(0);
#line 427
};

    //fprintf(six,"COMPUTESTART (after initialization) = %lu\n",gl->computestart);
    //fprintf(six,"COMPUTEEND = %lu\n",gl->computeend);
    //fprintf(six,"COMPUTETIME (after initialization) = %lu\n",gl->computeend-gl->computestart);
    //fprintf(six,"Measured Time (2nd timestep onward) = %lu\n",gl->tracktime);
    //fprintf(six,"Intramolecular time only (2nd timestep onward) = %lu\n",gl->intratime);
    //fprintf(six,"Intermolecular time only (2nd timestep onward) = %lu\n",gl->intertime);
    //fprintf(six,"Other time (2nd timestep onward) = %lu\n",gl->tracktime - gl->intratime - gl->intertime);

    printf("\nExited Happily with XTT = %g (note: XTT value is garbage if NPRINT > NSTEP)\n", XTT);

    
} /* main.c */

void WorkStart() /* routine that each created process starts at;
                    it simply calls the timestep routine */
{
    long ProcID;
    double LocalXTT;

    {	
#line 448
	while (barrier_wait(&(gl->start)) == 1);
#line 448
};

    { while(mutex_lock(&(gl->IndexLock)));};
    ProcID = gl->Index++;
    {mutex_unlock(&(gl->IndexLock));};

    {;};
    {;};
    {;};

    ProcID = ProcID % NumProcs;

      /*  POSSIBLE ENHANCEMENT:  Here's where one might bind processes to processors
        if one wanted to.
        */

    LocalXTT = MDMAIN(NSTEP,NPRINT,NSAVE,NORD1,ProcID);
    if (ProcID == 0) {
	    XTT = LocalXTT;
    }


    join_point(&myJoinPoint);

}

