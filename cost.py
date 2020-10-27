import subprocess
import os
from nearestneighbors import c_nearest_neighbors

class Costdata:
    def __init__(self,txt, metric):
        self.table_data = {
            '% time':[],
            'cumulative seconds':[],
            'self seconds': [],
            'calls': [],
            'self ms/call': [],
            'total ms/call': [],
            'name': []}

        lines = txt.splitlines()

        # get start index of table (+1 at the end to get first table row)
        start = 0
        while (not lines[start].split() or lines[start].split()[0] != 'time'):
            start+=1
        start+=1


        # get end index of table
        end = start
        while len(lines[end].split()) >= 1:
            end+=1

        # iterate over all table rows
        for i in range(start,end):
            values = [d.strip() for d in lines[i].split()]
            # some lines do not have 7 entries, ignored at the moment
            if (len(values) != 7):
                continue
            self.table_data['% time'].append(values[0])
            self.table_data['cumulative seconds'].append(float(values[1]))
            self.table_data['self seconds'].append(float(values[2]))
            self.table_data['calls'].append(int(values[3]))
            self.table_data['self ms/call'].append(float(values[4]))
            self.table_data['total ms/call'].append(float(values[5]))
            self.table_data['name'].append(values[6])

        self.metric_name = 'sim_eval' if 'sim_eval' in self.table_data['name'] else metric
        self.metric_index = self.table_data['name'].index(self.metric_name)
        self.metric_calls = self.table_data['calls'][self.metric_index]

    def print(self):
        data = self
        print("\nCost measurements: " + self.metric_name)
        print('{:<20s}{:d}'.format("function calls:", self.metric_calls))


def measure_costs(path, dataset, k, metric):
    # default 1 repetition for cost measurement
    nn_list, timing_data = c_nearest_neighbors(path, dataset, k, metric, 1,gprof_compile=True)

    process = subprocess.run(['gprof', os.path.join(path,"a.out"), '-p'], stdout=subprocess.PIPE, universal_newlines=True)
    return Costdata(process.stdout, metric)
