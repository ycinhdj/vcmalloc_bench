import json
from telnetlib import NOP
import traceback
from matplotlib import rcParams
import matplotlib.pyplot as plt
from numpy import log2, sqrt
from numpy import nan
import os
import numpy as np
from numpy import random
import pathlib
import pandas as pd
import matplotlib.ticker as mticker


#rcParams["figure.max_open_warning"] = 0
#xlabels = {
#    "Matrix Multiplication": "Square Matrices Sizes",
#    "CUDA": "Number of Parameters"
#}

rgb = ["red", "blue", "green", "violet", "yellow"]

markers = ["-+", "-x", "-X", "-", "s"]

base_allocator = "m"

scenarios = []
allocators = []
datatypes = []
sizes = []
tests = []
results = []

#fileNames = ['matmult_results.json', 'results.json']
#This is useful when performing tests in different PCs and then putting the results.json from other PCs in this fodlder
filenames = list(pathlib.Path('./').glob('*.json'))

for filename in filenames:
    # extract data
    with open(filename) as json_file:
        json_value = json.load(json_file)
        for scenario in json_value:
            scenarios.append(scenario)
            for allocator in json_value[scenario]:
                allocators.append(allocator)
                for datatype in json_value[scenario][allocator]:
                    datatypes.append(datatype)
                    for size in json_value[scenario][allocator][datatype]:
                        sizes.append(size)
                        for test in json_value[scenario][allocator][datatype][size]:
                            tests.append(test)
                            for result in json_value[scenario][allocator][datatype][size][test]:
                                results.append(result)


    # remove duplications
    scenarios = list(dict.fromkeys(scenarios))
    allocators = list(dict.fromkeys(allocators))
    datatypes = list(dict.fromkeys(datatypes))
    sizes = list(dict.fromkeys(sizes))
    tests = list(dict.fromkeys(tests))
    results = list(dict.fromkeys(results))

    # sort sizes
    sizes = sorted([int(i) for i in sizes])

    json_file = json.load(open(fileName))

    for datatype in datatypes:
        for test in tests:
            for result in results:
                # A new figure here
                # Bring me the scenarios to plot
                for scenario in scenarios:
                    df_dict = {}
                    # Fetch allocators
                    for allocator in allocators:
                        ys = []
                        xs = []
                        for size in sizes:
                            value = json_file.get(scenario, {}).get(allocator, {}).get(datatype, {}).get(str(size), {}).get(test, {}).get(result, None)
                            if value:
                                if scenario == "Matrix Multiplication":
                                    xs.append(int(size*100))
                                elif scenario == "CUDA":
                                    xs.append(int(size))
                                else:
                                    xs.append(int(log2(size)))
                                n = 0
                                accurate = 0
                                for i in value:
                                    n += 1
                                    accurate += float(value[i])
                                accurate /= n
                                ys.append(accurate)
                        if(ys):
                            df_dict[allocator] = ys
                            df_dict["sizes"] = xs
                    


                    if df_dict:
                            
                        # all allocators ready, plot them

                        #remove small sizes
                        if scenario == "Linked Data Structures":
                            for col in df_dict.keys():
                                df_dict[col] = df_dict[col][-6:]
                        elif scenario == "Matrix Multiplication":
                            for col in df_dict.keys():
                                df_dict[col] = df_dict[col][-5:]
                        elif scenario == "2D Matrix":
                            for col in df_dict.keys():
                                df_dict[col] = df_dict[col][-9:]
                        else:
                            for col in df_dict.keys():
                                df_dict[col] = df_dict[col][-10:]
                                                

                        df = pd.DataFrame(df_dict)
                        

                        #set the ratio to malloc
                        cols = [col for col in df.columns if col != "sizes" and col != "m" ]
                        for col in cols: 
                            df[col] = (df["m"] - df[col])/df["m"]
                        
                        

                        #get max_val to let malloc be 1% in the graph for visibility
                        max_val = 0
                        for col in df.columns:
                            if col != "sizes" and col != "m":
                                if df[col].abs().max() > max_val:
                                    max_val = df[col].abs().max()
                        df["m"] = max_val/100


                        df.set_index("sizes", inplace=True)
                        fig, axes = plt.subplots(nrows=2, gridspec_kw=dict(height_ratios=[3,1]))
                        df.plot(ax = axes[0], kind='bar', use_index = True, width=.8)
                        

                        #setting xlabels
                        if scenario == "CUDA":
                            axes[0].set_xlabel("numebr or parameters", fontsize=6)
                        elif scenario == "Matrix Multiplication":
                            axes[0].set_xlabel("Square Matrices Sizes", fontsize=6)
                        else:
                            axes[0].set_xlabel("sizes (Power of two)", fontsize=6)

                        axes[0].tick_params(axis='both', which='major', labelsize=6)
                        axes[0].set_ylabel(result, fontsize=6)
                        #plt.suptitle(scenario + " - " + test, size = 10)
                        axes[0].legend(loc = "best", fontsize="xx-small", ncol=3)

                        #add sign before number and % at the end
                        label_format = '{:+.0%}'
                        ticks_loc = axes[0].get_yticks().tolist()
                        axes[0].yaxis.set_major_locator(mticker.FixedLocator(ticks_loc))
                        axes[0].set_yticklabels([label_format.format(x) for x in ticks_loc])

                        #table
                        df['m'] = 0
                        rowslabels = df.columns.tolist()
                        colslabels = ['min', 'mean', 'max']
                        tabledata = []
                        for i in rowslabels:
                            tabledata.append(["{:+.0%}".format(x) for x in [df[i].min(), df[i].mean(), df[i].max()]])
                        axes[1].axis('off')
                        axes[1] = plt.table(
                            cellText=tabledata,
                            rowLabels=rowslabels,
                            colLabels=colslabels,
                            loc='center',
                            cellLoc='center',
                            colWidths=[.12 for x in colslabels])
                        axes[1].scale(1,1.7)    
                        axes[1].auto_set_font_size(False)
                        axes[1].set_fontsize(5)
                        

                        fig.set_size_inches(3.2, 2.5)
                        fig.tight_layout()
                        #plt.show()
                        
                        _scenario = scenario.replace(" ", "")
                        _datatype = datatype.replace("<", "").replace(">", "").replace(" ", "")
                        _test = test.replace(" ", "").replace(",", "").replace("+", "").replace("(", "").replace(")", "")
                        _result = result.replace(" ", "")

                        dir = "./" + _scenario + "/" + _datatype + "/" + _test + "/"
                        pathlib.Path(dir).mkdir(parents=True, exist_ok=True)
                        plt.savefig(dir + _result + ".pdf")

                        #for article
                        #dir2 = os.path.join("..", "..", "article", "figevals", _scenario, _datatype, _test, '')
                        #pathlib.Path(dir2).mkdir(parents=True, exist_ok=True)
                        #plt.savefig(dir2 + _result + ".pdf")

                        plt.close()
