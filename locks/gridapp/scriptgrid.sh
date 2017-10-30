#!/bin/sh
#########################################
#This script is for CS444 GRID assignment
#
#Run tests for time vs gridsize
#Written by Wenjin HU @clarkson.edu
#on 2/11/09
#
#You are encourged to modify or improve it
#########################################

#Thread number is fixed at 10
threadNr=10

#Grid size is changing from 2 to 10
gridSize=2

###########################################################
#each test will repeat 5 times
#the result time will be calculated by means and variance
#means will show the average of time usage
#variance will show the validation of running results 
###########################################################
repeatTimes=0


#For the test, it will be run at different granularity
#First at none level, then cell level, next row level, last is grid level
lockTypes="none cell row grid"

        ###############################################################
        ###############################################################
        #create and clean the file named raw.data
        #All the program display information will be stored in raw.data
        #If something wrong with the script, go to check raw.data 

        touch raw.data.grid

        ###############################################################
        ###############################################################


        ####################################
	#for loop for different granularity
	# none cell row grid
        ####################################

	for granularity in $lockTypes
	do
               #inner loop for different size of grid from 2*2 to 10*10
                echo "at $granularity granularity"
 		echo "------------------------------------------------"
		gridSize=2	
		
		#################################################
                #loop for trying different grid size  
		#################################################
  		until [ $gridSize = 11 ]
  		do
                	echo "Test:: Grid $gridSize Thread $threadNr"

                        ###############################
                        #repeate 5 times for each times
                        ###############################
			repeatTimes=0
			until [ $repeatTimes = 5 ]
                        do
                                ################################
                                #Run the program
                                #Display is saved in raw.data.grid
                                #################################
  				./gridapp $gridSize $threadNr -$granularity > raw.data
                cat raw.data >> raw.data.grid  
				cat raw.data | grep DATA 
				cat raw.data | grep Secs
				repeatTimes=$((repeatTimes+1))
			done
			echo "***********************************************"
			gridSize=$((gridSize+1))
		done
 	        echo "***********************************************"
	done