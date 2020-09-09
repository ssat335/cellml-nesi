# cellml-nesi project

## Summary
Performs fitting for mathematical models encoded in CellML (www.cellml.org), using the NeSI (www.nesi.org.nz) infrastructure.

## Contributors

* Shameer Sathar, Research Assistant, University of Auckland (Implementation, refactoring, owner), 2015.
* Mike Cooling, Systems Biomedicine Group, University of Auckland (concepts, virtual experiments, general design, minor bug fixes, test cases).

We are grateful to the Aotearoa Foundation for providing funds to develop this piece of scientific software.

## Performance improvement (population: 100, generations: 50):

|Cores|Runtime (mm:ss)|
|------|-------------|
|1|70:00|
|8|09:51|
|16|05:26|
|32|02:41|
|64|01:32|
|128|00:59|

## Building
### Required packages
#### Compiler
* ***mpiicpc***: an Intel MPI C++ compiler to compile the codebase. Load onto Pan Cluster's *build node* by:
```
module load intel/ics-2013
``` 

**Note**: You must be logged onto one of NeSI's "**build-nodes**" to load module and build projects

#### Libraries
* CellML-API
* AdvXMLParser

---

### Before you compile
The first thing you should do is to configure the include paths in **makefile**: *XMLPARSER_PATH* and *CELLML_PATH*.

Set *XMLPARSER_PATH* to point directly to the **AdvXMLParser** directory. 

Example:
```
XMLPARSER_PATH=/projects/uoa00322/AdvXMLParser
```

Configure *CELLML_PATH* to point directly to the **CellML-API** directory.

Example:
```
CELLML_PATH=/projects/uoa00322/cellml-sdk
```

---

### Compiling
Compile the project with the *makefile*:
```
make
```
which should generate the binary **cellml-fitter** in your current directory.

---

## Testing
Test the program with some supplied problems in the *tests* sub-directory:

Jump into any available test folder: 

Example: IP3 model
```
cd tests/3-ip3model
```

Feel free to look into the example test (XML) and job (Slurm) files.

Before batching any job with the example Slurm files, make sure you configure the library paths. Since CellML compiler relies on GCC compiler to build the code, LIBRARY_PATH and LD_LIBRARY_PATH should be configured by pointing to CellML **library** directory. For example:

```
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/projects/uoa00322/cellml-sdk/lib/
LIBRARY_PATH=$LIBRARY_PATH:/projects/uoa00322/cellml-sdk/lib/
```

Finally, batch the slurm job file. As an example, running `sbatch short.sl` will batch the example test *short.xml* on a single processor.

### Verbosity levels

The verbosity of the output can be set by adding or removing "-v" from the "srun" command

```
srun ../../cellml-fitter test.xml -v -v
```

|Number of "-v"s|Output|
|------|-------------|
|0|Only the best overall population member is reported|
|1|As 0 plus each generation's best is reported|
|2|As 1 plus each generation's members are reported|
|3|As 2 plus results of each genetic operator are reported and fitness evaluation by target point|
|4|As 3 plus detail of each genetic operator is reported, and detail of each target point evaluation|

## Creating your own 'Virtual Experiment'
Quickest way is by looking at an example: adapted from *short.xml* in IP3model problem...
```xml
<?xml version="1.0"?>
<CellMLTimeSeriesFit>
        <GA InitialPopulation="10" Generations="10" Mutation_proportion="0.4" Crossover_proportion="0.30" RNG="1">
                <Alleles>
                        <Allele Name="kf5" LowerBound="1.0e-8" UpperBound="9.999e2"/>
                        <Allele Name="kf4" LowerBound="1.0e-8" UpperBound="9.999e2"/>
                        ...
                        <Allele Name="Rpc" LowerBound="1.0e-2" UpperBound="5e3"/>
                </Alleles>
        </GA>
        <VirtualExperiments>
                <VirtualExperiment ModelFilePath="ip3model.cellml" Variable="IP3" ReportStep="50.0">
                <!-- note this VE doesn't have any Parameters to set, but next VE does (ie optional Parameters entity here) -->
                        <AssessmentPoints>
                                <AssessmentPoint time="100.0" target="0.026761882" />
                                <AssessmentPoint time="200.0" target="0.032711469" />
                                ...
                                <AssessmentPoint time="10000.0" target="0.015490316" />
                        </AssessmentPoints>
                </VirtualExperiment>
                <VirtualExperiment ModelFilePath="ip3model.cellml" Variable="Ca" ReportStep="50.0">
                        <Parameters>
                                <Parameter ToSet="Ls" Value="5.0"/>
                        </Parameters>
                        <AssessmentPoints>
                                <AssessmentPoint time="100.0" target="0.1" />
                                ...
                                <AssessmentPoint time="10000.0" target="0.099907954" />
                        </AssessmentPoints>
                </VirtualExperiment>
                <VirtualExperiment>
                        ...
                </VirtualExperiment>
                ...
                <VirtualExperiment>
                        ...
                </VirtualExperiment>
        </VirtualExperiments>
</CellMLTimeSeriesFit>
```

### ReportStep attributes

Each VirtualExperiment must have the ReportStep set. This is used by the simulator to force the integrator to report results at given time interval. If it is set to "0", it defaults to intervals of 1 time unit.

## MPI Notes

Load distribution is based on the workitems list. The list is populated with the item which contain
data to be computed and an opaque key, which is the context of the requestor. Once the list is fully
populated, call to process runs through the list requesting compute done on the data by sending the 
requests to the rest of the MPI ranks. Once the results are back, the observer function is called for
each result passing the workitem and the result to the callback.

## Fitness of a Virtual Experiment Group

Fitness evaluation currently treats every target data point as of equal importance. This will
result in biasing the fits to those experiments with more data targets, unless each experiment
has the same number of data targets. If this becomes a problem in practice, we could implement
a simple weighing system to resolve it.

