# resmerge - Resolution Level Clusterings Merger with Filtering

Merges multiple cluster resolution / hierarchy levels into the single collection, i.e. flattens the input hierarchy / resolutions specified by the files and / or directories. Only unique clusters independent of the nodes order are saved into the output file with optional filtering by the clusters size.  
`resmerge` is one of the utilities designed for the [PyCaBeM](https://github.com/eXascaleInfolab/PyCABeM) clustering benchmark.

## Content
- [Deployment](#deployment)
	- [Dependencies](#dependencies)
	- [Compilation](#compilation)
- [Usage](#usage)
- [Related Projects](#related-projects)

# Deployment

## Dependencies
There no any dependencies for the execution or compilation.  
However, to extend the input options and automatically regenerate the input parsing,
[*gengetopt*](https://www.gnu.org/software/gengetopt) application should be installed: `$ sudo apt-get install gengetopt`.

## Compilation
Just execute `$ make`.  
To update/extend the input parameters modify `args.ggo` and run `GenerateArgparser.sh` (calls `gengetopt`).

# Usage
Execution Options:
```
$ ./resmerge -h
resmerge 1.0

Merges multiple clusterings on multiple resolutions into the single set of
clusters with optional filtering.

Usage: resmerge [OPTIONS] clusterings

  clusterings  - clusterings specified by the given files and directories
(non-recursive traversing)

  -h, --help              Print help and exit
  -V, --version           Print version and exit
  -o, --output=STRING     output file name. If a single directory <dirname> is
                            specified then the default output file name is
                            <dirname>.cnl  (default=`clusters.cnl')
  -r, --rewrite           rewrite already existing resulting file or skip the
                            processing  (default=off)
  -b, --btm-size=LONG     bottom margin of the cluster size to process
                            (default=`0')
  -t, --top-size=LONG     top margin of the cluster size to process
                            (default=`0')
  -m, --membership=FLOAT  average expected membership of nodes in the clusters,
                            > 0, typically >= 1  (default=`1')
```

# Related Projects
- [GenConvNMI](https://github.com/eXascaleInfolab/GenConvNMI) - Overlapping NMI evaluation that is compatible with the original NMI and suitable for both overlapping and multi resolution (hierarchical) clustering evaluation.
- [ExecTime](https://bitbucket.org/lumais/exectime/)  - A lightweight resource consumption profiler.
- [PyCABeM](https://github.com/eXascaleInfolab/PyCABeM) - Python Benchmarking Framework for the Clustering Algorithms Evaluation. Uses extrinsic (NMIs) and intrinsic (Q) measures for the clusters quality evaluation considering overlaps (nodes membership by multiple clusters).
