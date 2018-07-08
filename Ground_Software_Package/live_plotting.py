import datetime
import os
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
import matplotlib.dates as mdates
from mpl_toolkits.mplot3d import Axes3D
img = ""
img_large = ""
header_data = "dummy"
fig = ""
axes = ""
fig_a = fig_g = fig_m = ""
ax_a = ax_g = ax_m = ""

def set_up_plots():
    '''
    set the the axes of the plots that are desired

    Written by Curtis Puetz 2018-07-07

    :return: None
    '''
    global img, img_large
    plt.figure(1)
    plt.title('Interior Temperature Profile')
    plt.xlabel('Temperature ($\degree$C)')
    plt.ylabel('Altitude (m)')
    plt.figure(2)
    plt.title('Altitude')
    plt.xlabel('Time')
    plt.ylabel('Altitude')
    plt.figure(3)
    plt.title('Connection Strength')
    plt.xlabel('Time')
    plt.ylabel('RSSI')
    global fig, axes
    fig, axes = plt.subplots(3, 3, figsize=(18, 9), num=4, sharex=True)
    fig.suptitle('IMU Sensor Suite (Whole Mission)')
    axes[0, 0].set_title('Accel')
    axes[0, 1].set_title('Gyro')
    axes[0, 2].set_title('Magnet')
    #axes[0, 3].set_title('Gravity')
    axes[0, 0].set_ylabel('X')
    axes[1, 0].set_ylabel('Y')
    axes[2, 0].set_ylabel('Z')
    plt.figure(5)
    plt.title('Number of GPS Satellites used')
    plt.xlabel('Time')
    plt.ylabel('Number Satellites')
    # load plot and get ride of white spots
    plt.figure(6)
    plt.title('Location of payload')
    img = mpimg.imread(r'C:\Users\puetz\Downloads\real_wingham_sc.PNG')
    imgplot = plt.imshow(img)
    plt.figure(7)
    plt.title('Location of payload')
    img1 = mpimg.imread(r'C:\Users\puetz\Downloads\large_wingham.PNG')
    imgplot1 = plt.imshow(img1)
    # these while loop may only be needed for specific png files
    # i = 0
    # while not (img[i][0] == np.array([1, 1, 1, 1])).all():
    #     i += 1
    # img = img[:i]
    # i = 0
    # while not (img[0][i] == np.array([1, 1, 1, 1])).all():
    #     i += 1
    # img = img[:, :i]
    # plot the image
    plt.figure(8)
    plt.title('Counts #1')
    plt.xlabel('Time')
    plt.ylabel('Counts')
    plt.figure(9)
    plt.title('Counts #2')
    plt.xlabel('Time')
    plt.ylabel('Counts')
    plt.figure(10)
    plt.title('Simultaneous Counts')
    plt.xlabel('Time')
    plt.ylabel('Counts')
    plt.figure(11)
    plt.title('Counts #1 Altitude Profile')
    plt.xlabel('Counts')
    plt.ylabel('Altitude (m)')
    plt.figure(12)
    plt.title('Counts #2 Altitude Profile')
    plt.xlabel('Counts')
    plt.ylabel('Altitude (m)')
    plt.figure(13)
    plt.title('Simultaneous Counts Altitude Profile')
    plt.xlabel('Counts')
    plt.ylabel('Altitude')
    global fig_a, fig_g, fig_m, ax_a, ax_g, ax_m
    fig_a = plt.figure(14)
    ax_a = fig_a.add_subplot(111, projection='3d')
    ax_a.set_xlim([-1, 1])
    ax_a.set_ylim([-1, 1])
    ax_a.set_zlim([-1, 1])
    ax_a.set_xlabel('X')
    ax_a.set_ylabel('Y')
    plt.title('Accelerometer Direction')
    fig_g = plt.figure(15)
    ax_g = fig_g.add_subplot(111, projection='3d')
    ax_g.set_xlim([-1, 1])
    ax_g.set_ylim([-1, 1])
    ax_g.set_zlim([-1, 1])
    ax_g.set_xlabel('X')
    ax_g.set_ylabel('Y')
    plt.title('Gyroscope Direction')
    fig_m = plt.figure(16)
    ax_m = fig_m.add_subplot(111, projection='3d')
    ax_m.set_xlim([-1, 1])
    ax_m.set_ylim([-1, 1])
    ax_m.set_zlim([-1, 1])
    ax_m.set_xlabel('X')
    ax_m.set_ylabel('Y')
    plt.title('Magnetometer Direction')

def plot_data(data, header_data):
    '''
    plot a single data point for each of the plots set up in 'set_up_plots()' function.
    This will be called each time a comma separated data list is recieved (so long as a
    header data list has already been recieved)

    Written by Curtis Puetz 2018-07-07

    :param data: the list of data generated from the downlinked comma separated data list
    :return: None
    '''
    pi_time = datetime.datetime.strptime(data[0], '%Y%m%d_%X.%f')

    # isolate the floats and save them in a dictionary (while checking the units of altitude)
    data = data[1:]
    header_data = header_data[1:]
    data_dict = dict(zip(header_data, data))
    if data_dict['Altu'] == "KM":
        alt_factor = 1000
    else:
        alt_factor = 1
    data_dict['Alt'] *= alt_factor
    del data_dict['Altu']
    del data_dict['NS']
    del data_dict['EW']
    data_float = [[] for i in range(len(data_dict))]
    for i, dai in enumerate(list(data_dict.values())):
        if dai == "":
            data_float[i] = ""
        else:
            data_float[i] = float(dai)
    data_dict = dict(zip(list(data_dict.keys()), data_float))
    # now plot the data
    def plot_time_x(thedata, fig):
        plt.figure(fig)
        plt.plot([], [])
        plt.scatter(pi_time, thedata, color='blue')
        plt.gcf().autofmt_xdate()
        myFmt = mdates.DateFormatter('%H:%M:%S')
        plt.gca().xaxis.set_major_formatter(myFmt)
    def plot_y_x(xdata, ydata, fig):
        plt.figure(fig)
        plt.scatter(xdata, ydata, color='blue')
    if not data_dict['TC'] == "" and not data_dict['Alt'] == "":
        plot_y_x(data_dict['TC'], data_dict['Alt'], 1)
    if not data_dict['Alt'] == "":
        plot_time_x(data_dict['Alt'], 2)
    if not data_dict['RSSI'] == "":
        plot_time_x(data_dict['RSSI'], 3)
    # accelerometer data
    global fig, axes
    def plot_time_x_sub(thedata, ax):
        ax.plot([], [])
        ax.scatter(pi_time, thedata, color='blue')
    if not data_dict['Acxms2'] == "":
        plot_time_x_sub(data_dict['Acxms2'], axes[0, 0])
    if not data_dict['Acyms2'] == "":
        plot_time_x_sub(data_dict['Acyms2'], axes[1, 0])
    if not data_dict['Aczms2'] == "":
        plot_time_x_sub(data_dict['Aczms2'], axes[2, 0])
    if not data_dict['Gyxrs'] == "":
        plot_time_x_sub(data_dict['Gyxrs'], axes[0, 1])
    if not data_dict['Gyyrs'] == "":
        plot_time_x_sub(data_dict['Gyyrs'], axes[1, 1])
    if not data_dict['Gyzrs'] == "":
        plot_time_x_sub(data_dict['Gyzrs'], axes[2, 1])
    if not data_dict['MgxuT'] == "":
        plot_time_x_sub(data_dict['MgxuT'], axes[0, 2])
    if not data_dict['MgyuT'] == "":
        plot_time_x_sub(data_dict['MgyuT'], axes[1, 2])
    if not data_dict['MgzuT'] == "":
        plot_time_x_sub(data_dict['MgzuT'], axes[2, 2])
    # if not data_dict['Gvxms2'] == "":
    #     plot_time_x_sub(data_dict['Gvxms2'], axes[0, 3])
    # if not data_dict['Gvyms2'] == "":
    #     plot_time_x_sub(data_dict['Gvyms2'], axes[1, 3])
    # if not data_dict['Gvzms2'] == "":
    #     plot_time_x_sub(data_dict['Gvzms2'], axes[2, 3])
    for ax in fig.axes:
        plt.sca(ax)
        plt.xticks(rotation=30)
    if not data_dict['Nsat'] == "":
        plot_time_x(data_dict['Nsat'], 5)
    # Longitude and Latitude map
    if not data_dict['LtDgMn'] == "" and not data_dict['LnDgMn'] == "":
        plt.figure(6)
        # kingston
        # left_long = -76.48390
        # right_long = -76.45510
        # top_lat = 44.23739
        # bottom_lat = 44.22415
        # wingham
        # left_long = -81.41451
        # right_long = -80.43596
        # top_lat = 43.96803
        # bottom_lat = 43.66735
        # real wingham
        left_long = --81.41625
        right_long = -80.45979
        top_lat = 43.60416
        bottom_lat = 43.96321
        lat = int(data_dict['LtDgMn']/100) + (data_dict['LtDgMn'] - int(data_dict['LtDgMn']/100)*100)/60
        long = -(int(data_dict['LnDgMn']/100) + (data_dict['LnDgMn'] - int(data_dict['LnDgMn']/100)*100)/60)
        index_y = np.interp(lat, np.linspace(bottom_lat, top_lat, len(img)), np.arange(0, len(img))[::-1])
        index_x = np.interp(long, np.linspace(left_long, right_long, len(img[0])), np.arange(0, len(img[0])))
        plt.scatter(index_x, index_y, color='blue', s=20)
        # large wingham
        plt.figure(7)
        left_long = -82.14579
        right_long = -79.30585
        top_lat = 44.51928
        bottom_lat = 43.25126
        lat = int(data_dict['LtDgMn']/100) + (data_dict['LtDgMn'] - int(data_dict['LtDgMn']/100)*100)/60
        long = -(int(data_dict['LnDgMn']/100) + (data_dict['LnDgMn'] - int(data_dict['LnDgMn']/100)*100)/60)
        index_y = np.interp(lat, np.linspace(bottom_lat, top_lat, len(img)), np.arange(0, len(img))[::-1])
        index_x = np.interp(long, np.linspace(left_long, right_long, len(img[0])), np.arange(0, len(img[0])))
        plt.scatter(index_x, index_y, color='blue', s=20)
    # counts vs time and altitude profiles
    if not data_dict['C1'] == "":
        plot_time_x(data_dict['C1'], 8)
    if not data_dict['C2'] == "":
        plot_time_x(data_dict['C2'], 9)
    if not data_dict['SC'] == "":
        plot_time_x(data_dict['SC'], 10)
    if not data_dict['C1'] == "" and not data_dict['Alt'] == "":
        plot_y_x(data_dict['C1'], data_dict['Alt'], 11)
    if not data_dict['C2'] == "" and not data_dict['Alt'] == "":
        plot_y_x(data_dict['C2'], data_dict['Alt'], 12)
    if not data_dict['SC'] == "" and not data_dict['Alt'] == "":
        plot_y_x(data_dict['SC'], data_dict['Alt'], 13)
    # plot direction of IMU stuff
        global fig_a, fig_g, fig_m, ax_a, ax_g, ax_m
    def plot_directions(fig, ax, x, y, z, title):
        if not data_dict[x] == "" and not data_dict[y] == "" and not data_dict[z] == "":
            U = data_dict[x]
            V = data_dict[y]
            W = data_dict[z]
            unit_length = np.linalg.norm([U, V, W])
            U = U / unit_length
            V = V / unit_length
            W = W / unit_length
            ax.clear()
            ax = fig.add_subplot(111, projection='3d')
            ax.set_xlim([-1, 1])
            ax.set_ylim([-1, 1])
            ax.set_zlim([-1, 1])
            ax.set_xlabel('X')
            ax.set_ylabel('Y')
            ax.set_title(title)
            ax.quiver(0, 0, 0, U, V, W)
    plot_directions(fig_a, ax_a, 'Acxms2', 'Acyms2', 'Aczms2', 'Accelerometer Direction')
    plot_directions(fig_g, ax_g, 'Gyxrs', 'Gyyrs', 'Gyzrs', 'Gyroscope Direction')
    plot_directions(fig_m, ax_m, 'MgxuT', 'MgyuT', 'MgzuT', 'Magnetometer Direction')
    plt.pause(0.05)

def read_last_line_in_data_log():
    """
    This function will read the last line in the data log file and return it

    Written by Daniel Letros, 2018-07-03

    :return: None
    """
    timestamp = datetime.datetime.utcnow().strftime("%Y%m%d")
    log_file_path = r"C:\Users\puetz\Desktop\Telemtry_logs"
    log_file_path += os.sep + timestamp
    file_name = log_file_path + os.sep + timestamp + "_data.txt"
    # file_name = r'C:\Users\puetz\Desktop\Telemtry_logs\Test\test.txt' # test generated data
    # file_name = r'C:\Users\puetz\Desktop\litest.txt'
    try:
        with open(file_name, 'rb') as f:
            f.seek(-2, os.SEEK_END)
            while f.read(1) != b'\n':
                f.seek(-2, os.SEEK_CUR)
            content = f.readline().decode()
    except:
        with open(file_name, 'rb') as f:
            content = f.readlines()[-1].decode()
    return content

'''
note to users: 

1) you must hard code in the location you want to LOAD from your log files within the 
read_last_line_in_data_log() function on the line:
log_file_path = r"C:\\Users\puetz\Desktop\Telemtry_logs"

2) the 'plot_pause_for_interactive' variable should be a little bit faster than the rate of 
data being written to the log files in serial_communication.py (i.e. 'check_for_data_delay'
variable). It can easily be 1 second. This allows for CPU usage to remain low, since the program
does not check the log file as often.

3) you must be generating data for this program to do anything. So either serial_communication.py
needs to be running and recieving data from the balloon, or generate_dummy_logs.py needs to be
running to generate artifical data. In the latter, a text file needs to be supplied to 
generate_dummy_logs.py with reasonable data, and the log_file_paths in both 'generate_dummy_logs.py'
and this program need to be appropriate.

4) the png files used for the longitude latitude maps needs to be set to your location (you also
need to generate the constrains of the picture manually)
'''
if __name__ == '__main__':
    plot_pause_for_interactive = 1
    set_up_plots()
    plt.ion()
    hold = ""
    while True:
        data = read_last_line_in_data_log()
        if data == hold:
            plt.pause(plot_pause_for_interactive)
            continue
        hold = data
        data = data[:-2]
        #print(data)
        if data[0] == "P":
            header_data = data.split(',')
        elif data[0] == '2':
            data = data.split(',')


        if not header_data == "dummy" and data[0][0] == '2':
            #print('A')
            plot_data(data, header_data)



