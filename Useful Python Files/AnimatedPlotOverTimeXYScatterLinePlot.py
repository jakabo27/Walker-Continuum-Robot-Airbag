#
# This file creates an animated plot from a set of data.  
# The time is written to the title each frame, and it freezes the final frame at the end 
#
#

%matplotlib inline

import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
import os
import sys
from shutil import copyfile
from matplotlib import rc
from IPython.display import HTML
from matplotlib import animation

basePath = "G:/My Drive/Documents/SCHOOL/Thesis/Videos/"
walkerLogFileName = 'Test31Log'
resultsPath = "G:/My Drive/Documents/SCHOOL/Thesis/RESULTS/"

logFileSource = basePath + walkerLogFileName + ".csv"
saveFileName =  walkerLogFileName  # Base name to use for saving the file
plotTitleText = walkerLogFileName  # Plot title
TestNum = 1

# Set the font to Type 42 (aka Type 1) to match IEEEtrans
plt.rcParams['pdf.fonttype'] = 42
plt.rcParams['ps.fonttype'] = 42

# Create a figure and axes
fig = plt.figure(figsize=(1920/300,1080/300), dpi=300, facecolor='w');
ax1 = plt.subplot(1,1,1)  ; 

# set up the subplots as needed
ax1.set_xlim(( 0, 1250));            
ax1.set_ylim((0, 1000));
ax1.set_xlabel('Time [ms]');
ax1.set_ylabel('Encoder Value');
ax1.set_facecolor('w');
txt_title = ax1.set_title(plotTitleText)

# create objects that will change in the animation. These are
# initially empty, and will be given new values for each frame
# in the animation.
line1, = ax1.plot([], [], 'r', lw=2); # ax.plot returns a list of 2D line objects
line2, = ax1.plot([], [], 'g', lw=2);
line3, = ax1.plot([], [], 'b', lw=2);
line4, = ax1.plot([], [], 'y', lw=2);
line5, = ax1.plot([], [], 'k', lw=2); 

ax1.legend(['Encoder 1','Encoder 2','Encoder 3','Encoder 4', 'Shaft'],prop={'size': 9}, loc='upper left');

# Import data
df = pd.read_csv(logFileSource)
#       Note - my file has columns timeSinceDeploy, E1, E2, E3, E4, Shaft.  
#           timeSinceDeploy is the time in milliseconds.  All others are Y-values to plot. 
# df.head(5)

# animation function. This is called sequentially by FuncAnimation
def drawframe(n):
    
    newN = n        # Increment frame

    if(n >= 400):
        newN = 400  # Keep the final frame frozen at the end (frames 400-600)
         
    x = (df["timeSinceDeploy"][0:newN])
    y1 = df["E1"][0:newN]
    y2 = df["E2"][0:newN]
    y3 = df["E3"][0:newN]
    y4 = df["E4"][0:newN]
    y5 = df["Shaft"][0:newN]*-1

    line1.set_data(x, y1)
    line2.set_data(x, y2)
    line3.set_data(x, y3)
    line4.set_data(x, y4)
    line5.set_data(x, y5)
    
    # Set the title to display the test number and current time
    txt_title.set_text('Test ' + str(TestNum)+ '\nTime = {0:4d}ms'.format(newN*5))
    return (line1,line2)

# Make the animation
anim = animation.FuncAnimation(fig, drawframe, frames=600, interval=5, blit=True, repeat_delay=500)
animForHTML = anim  # Make a copy so we don't mess it up
rc('animation', html='html5')

# Preview it
HTML(animForHTML.to_html5_video())

# # Save the last frame as an image
# fig.savefig(basePath + saveFileName + ' LogFinalFrame.pdf', bbox_inches='tight')
# fig.savefig(basePath + saveFileName +' LogFinalFrame.jpg', bbox_inches='tight')

# #Save as MP4 video in real-time
# anim.save(basePath + saveFileName + ' Log.mp4', writer = animation.FFMpegWriter(fps = 200))

# Save as MP4 in 25% speed (slow-mo)
# anim.save(basePath + saveFileName +' Log_SLOW.mp4', writer = animation.FFMpegWriter(fps = 50))
