import matplotlib.pyplot as plt
import numpy as np


threads = np.array([1, 2, 3, 4, 6 , 8, 16])

#POWER
power_tm = np.array([382265, 250582, 234329, 254457,277041, 281051, 337160])
power_lock = np.array([365175, 241141, 230871, 252061,272281, 280883, 356899])
power_single = 133164

#Lukas Computer
lukas_tm = np.array([831798, 639659, 543506 ,496861, 678816, 186525, 490811])
lukas_lock = np.array([501019 ,334728 ,277076,271972,266391, 259738, 253789])
lukas_single = 205356

plt.plot(threads, 1000000000/power_tm, 'r')
plt.plot(threads , 1000000000/power_lock, 'g')
plt.ylabel('Operations/ Microseconds')
plt.xlabel('Number Of Threads')
plt.title('POWER8 Performance Result')
plt.legend(['Transactional Memory', 'Lock'])
plt.show()

plt.plot(threads, 1000000000/lukas_tm, 'r')
plt.plot(threads , 1000000000/lukas_lock, 'g')
plt.ylabel('Operations/ Microseconds')
plt.xlabel('Number Of Threads')
plt.title(' Intel i3-6006U Performance Result')
plt.legend(['Transactional Memory', 'Lock'])

plt.show()
