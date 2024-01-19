import nettensorpat.Tensor_Python as Tensor_Python
from typing import Literal, Optional, TypedDict
from nettensorpat.Typing import Default

class NetTensorPat:
    DEFAULT = Default
    
    def frequentClustering(
        geneTotal: int,
        networkListFile: str,
        maxNode: int = Default.MAXGENE,
        mute: bool = False,
        local: bool = False,
        seedNode: int = Default.InitXYBy.DEFAULT,
        maxPattern: int = Default.MAXPATTERN,
        nIteration: int = Default.NITERATION,
        nStage: int = Default.NSTAGE,
        minNode: int = Default.MINGENE,
        minNetwork: int = Default.MINNET,
        minDensity: float = Default.MIN_DENSITY,
        maskStrategy: Literal["EDGES_PATTERN", "EDGES_ALLNETS", "GENES"] = Default.MaskStrategyName.DEFAULT,
        overlapPattern: Literal["PATTERN_WITH_NONZEROS_XY", "PATTERN_WITH_MORE_NETS", "PATTERN_WITH_MORE_GENES", "PATTERN_WITH_BOTH"] = Default.OverlapPattern.DEFAULT,
        nEdgesLoad: int = Default.NEDGES_LOAD,
        loadUnweighted: bool = Default.LOAD_UNWEIGHTED,
        resume: bool = Default.RESUME,
        excludeEdges: bool = Default.EXCLUDE_EDGES,
        resultFilePrefix: str = Default.Path.RESULTFILE_PREFIX,
        networkFileSuffix: str = Default.Path.DATAFILE_SUFFIX,
        networksPath: str = Default.Path.NETWORKS_FOLDER,
        resultsPath: str = Default.Path.RESULT,
        level: Literal[1, 2, 3] = Default.LEVEL,
    ) -> bool:
        return(Tensor_Python.frequentClustering(
            geneTotal,
            networkListFile,
            maxNode,
            mute,
            local,
            seedNode,
            maxPattern,
            nIteration,
            nStage,
            minNode,
            minNetwork,
            minDensity,
            maskStrategy,
            overlapPattern,
            nEdgesLoad,
            loadUnweighted,
            resume,
            excludeEdges,
            resultFilePrefix,
            networkFileSuffix,
            networksPath,
            resultsPath,
            level
        ))
