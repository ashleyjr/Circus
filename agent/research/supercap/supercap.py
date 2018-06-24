import matplotlib.pyplot as plt

# CC discharge
# V(t) = v0 - ((t*i)/C)

cc = 0.01               # Amps
v_0 = 5                 # Volts
c = [1, 10, 100, 220]   # mF
t = range(0, 100000)    # milliseconds

for i in c:
    v = []
    for j in t:
        v.append(v_0 - (((j/float(1000))/(i/float(1000)))*cc))

    plt.semilogx(t, v)
plt.ylabel('v(t) (voltage)')
plt.xlabel('time (milli seconds)')
plt.axis([0, t[-1], 0, v_0])
plt.grid(True)
plt.show()
