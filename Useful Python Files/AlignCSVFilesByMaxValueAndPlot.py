#
#   This file takes in a series of CSV files (data1.csv, data2.csv, data3.csv etc.) and plots them
#   where they are aligned by each dataset's max value. 
#
#   We used this to make a plot of 5 fall tests with g-force - the peak g-force occured at the same
#   physical moment for each test (dummy hit ground) but was a different timestamp based on when the 
#   data recording was started.  
#

import numpy as np
import matplotlib.pyplot as plt
# import seaborn as sns
import pandas as pd
import os, sys, statistics, ffmpeg
from shutil import copyfile
from math import sqrt

# Import Physics Toolbox data
# Name the files "myDataFile 1.csv" through "myDataFile 6.csv" etc.  
basePath = "G:/My Drive/Documents/SCHOOL/Thesis/DataFiles/"
resultsPath = basePath

# The name of the files to get data from - ex. dataFile 1.csv, dataFile 2.csv, etc. 
fileNaming = "dataFile "
fileStartNum = 1    # Number they start on - in case you want to plot files 4-8
numFiles = 5
fileNumbers = np.arange(fileStartNum, fileStartNum + numFiles) # create array like [1 2 3 4 5]
plotTitleText = 'Example Falls Tests'  # Title of the figure

# Open the first file to make sure you set up your path right
df1Name = basePath + fileNaming + str(fileStartNum) + ".csv"
df1 = pd.read_csv(df1Name)
print(df1.head())
print("")

# # Set the Font Type to 42 (instead of Type 3) to avoid IEEE conference issues with PDF figure
plt.rcParams['pdf.fonttype'] = 42
plt.rcParams['ps.fonttype'] = 42


# Column header name to plot - for example, one of my columns is 'TgF' (total g force)
columnNameToPlot = 'TgF' 

# How many data points before and after the max to plot
prevValues = 200
afterValues = 350


# Base the X-axis on the first data file's time column
df1StartRow = df1[columnNameToPlot].idxmax() # Row number with the max value
startRow = df1StartRow
df1FullSlice = df1.iloc[startRow-prevValues:startRow+afterValues] 
df1TimeShifted = df1FullSlice['time'] - df1FullSlice['time'][df1StartRow]
xVals = df1TimeShifted.to_numpy() * 1000  # Convert seconds to milliseconds


# Create figure 
fig1 = plt.figure(figsize=(1920/300,1080/300), dpi=300, facecolor='w')
ax1 = plt.subplot(1,1,1)
FallType = "Fall "       # Prefix in the legend

j = 1
maxValues = []
for dfNum in np.ndenumerate(fileNumbers):
    dfName = basePath + fileNaming + str(dfNum[1]) + ".csv" # File name
    df = pd.read_csv(dfName)                                # Read in this csv file
    
    # Calculate and add new Totals columns if it doesn't include it
#     df['accelTotal'] = (df['ax']**2  + df['ay']**2 + df['az']**2)**0.5
#     df['gforceTotal'] = (df['gFx']**2  + df['gFy']**2 + df['gFz']**2)**0.5

    
    # Plot the main data points
    thisMaxAccel = df[columnNameToPlot].max(axis=0)
    maxValues.append(thisMaxAccel)                     # Save the max value for the legend
    startRow = df[columnNameToPlot].idxmax()           # Row number with the max value
    thisSlice = df.iloc[startRow-prevValues:startRow+afterValues] # Get the data to plot
    yVals = thisSlice[columnNameToPlot].to_numpy()
    thisLabel = FallType + " " + str(dfNum[1]) + " (%.1f"%(thisMaxAccel) + " g's)"
    ax1.plot(xVals, yVals, '--o', label=thisLabel,  markersize=1, linewidth=0.5)
    
    # Print the min/max value of each column
    maxX = df['gFx'].max(axis=0)
    maxY = df['gFy'].max(axis=0)
    maxZ = df['gFz'].max(axis=0)
    minX = df['gFx'].min(axis=0)
    minY = df['gFy'].min(axis=0)
    minZ = df['gFz'].min(axis=0)
    print("Max X:  %.1f/%.1f \t Max Y: %.1f/%.1f \t Max Z: %.1f/%.1f"%(minX,maxX, minY,maxY, minZ,maxZ))
    
    j+=1
    
# Finalize figure 
# ax1.set_title(plotTitleText)
ax1.legend(loc = 'upper right', prop={'size': 10})
ax1.set_ylabel("G-Force [g's]")
ax1.set_xlabel("Time From Peak [ms]")
ax1.set_xlim(xVals[0], xVals[-1])
ax1.set_ylim((0,20));

plt.show()

# Save Figure in various formats
# fig1.savefig(resultsPath + '' + fileNaming + 'RESULTS.pdf', bbox_inches='tight') # PDF for Overleaf
# fig1.savefig(resultsPath + '' + fileNaming + 'RESULTS.jpg', bbox_inches='tight') # JPG for easy browsing
# fig1.savefig(resultsPath + '' + fileNaming + 'RESULTS.svg', bbox_inches='tight') # SVG for Powerpoint