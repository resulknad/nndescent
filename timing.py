import numpy as np

class Timingdata:
    def __init__(self,c, t, method):
        self.method = method

        self.cycles = c
        if c is not None:
            self.median_cycle = np.median(c)
            self.avg_cycle = np.average(c)
            self.std_cycle = np.std(c)
            self.var_cycle = np.var(c)

        self.runtime = t
        self.median_runtime = np.median(t)
        self.avg_runtime = np.average(t)
        self.std_runtime = np.std(t)
        self.var_runtime = np.var(t)

    def save(self, filename):
        np.savetxt(filename, self.X)

    def print(self):
        data = self
        print("Performance measurement: "+ data.method + " (using " + str(data.runtime.shape[0]) + " repetitions)")
        if data.cycles is not None:
            print("\ncycles:")
            print('{:<20s}{:<10s}'.format("average:", str(data.avg_cycle)))
            print('{:<20s}{:<10s}'.format("median:", str(data.median_cycle)))
            print('{:<20s}{:<10s}'.format("std deviation:", str(data.std_cycle)))
            print('{:<20s}{:<10s}'.format("variance:", str(data.var_cycle)))

        print("\nruntime (in seconds):")
        print('{:<20s}{:<10s}'.format("average:",str(data.avg_runtime)))
        print('{:<20s}{:<10s}'.format("median:" ,str(data.median_runtime)))
        print('{:<20s}{:<10s}'.format("std deviation:", str(data.std_runtime)))
        print('{:<20s}{:<10s}'.format("variance:", str(data.var_runtime)))

def parse_output(txt):
    if (txt[len(txt)-4].split()[-1] != 'finished'):
        print("Algorithm not finished, probably some error occured.")
    c = float(txt[len(txt)-3].split()[0])
    t = float(txt[len(txt)-2].split()[0])
    return c,t
