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
resmerge 1.1

Merge multiple clusterings (resolution/hierarchy levels) with optional
filtering of clusters by size and nodes filtering by base.

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
  -m, --membership=FLOAT  average expected membership of the nodes in the
                            clusters, > 0, typically >= 1  (default=`1')

 Mode: sync
  Synchronize the node base of the merged clustering
  -s, --sync-base=STRING  synchronize node base with the specified collection

 Mode: exrtact
  Extract the node base from the specified clustering(s)
  -e, --extract-base      extract the node base from the clusterings instead of
                            merging the clusterings  (default=off)
```

**Examples**
Merge clusterings (resolution levels) from the `<dirname>` to `<dirname>.cnl`:
```
$ ./resmerge  /opt/tests/tmp/resolutions
```
Extract node base to `<filename>_base.cnl`:
```
./resmerge -e  /opt/tests/collection.cnl
```
Merge clusterings, synchronize the node base and output resulting flattened hierarchy/levels to the specified file:
```
$ ./resmerge -s /opt/tests/levels_nodebase.cnl -o /opt/tests/flatlevs_synced.cnl /opt/tests/levels/ /opt/tests/level_extra.cnl
```

# Related Projects
- [GenConvNMI](https://github.com/eXascaleInfolab/GenConvNMI) - Overlapping NMI evaluation that is compatible with the original NMI and suitable for both overlapping and multi resolution (hierarchical) clustering evaluation.
- [ExecTime](https://bitbucket.org/lumais/exectime/)  - A lightweight resource consumption profiler.
- [PyCABeM](https://github.com/eXascaleInfolab/PyCABeM) - Python Benchmarking Framework for the Clustering Algorithms Evaluation. Uses extrinsic (NMIs) and intrinsic (Q) measures for the clusters quality evaluation considering overlaps (nodes membership by multiple clusters).

**Note:** Please, [star this project](https://github.com/eXascaleInfolab/resmerge) if you use it.
