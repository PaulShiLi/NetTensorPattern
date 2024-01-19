class _InitXYBy:
    ONES: int = 0
    RAND: int = -1
    UNIT: int = -2
    DEFAULT: int = ONES

class _MaskStrategyName:
    EDGES_PATTERN: str = "EDGES_PATTERN"
    EDGES_ALLNETS: str = "EDGES_ALLNETS"
    GENES: str = "GENES"
    DEFAULT: str = EDGES_PATTERN

class _OverlapPattern:
    NONZEROS: str = "PATTERN_WITH_NONZEROS_XY"
    MORE_NETS: str = "PATTERN_WITH_MORE_NETS"
    MORE_GENES: str = "PATTERN_WITH_MORE_GENES"
    BOTH: str = "PATTERN_WITH_BOTH"
    DEFAULT: str = BOTH

class _Path:
    RESULTFILE_PREFIX: str = ""
    RESULT: str = "./results"
    DATASET_DEBUG_LIST: str = "./debug/debug_datasets"
    NETWORKS_FOLDER: str = "./datasets"
    DATAFILE_SUFFIX: str = ".sig"

class Default:
    NPATTERN_UNLIMITED: int = -1
    MAXPATTERN: int = NPATTERN_UNLIMITED
    
    NITERATION: int = 20
    NSTAGE: int = 20
    
    MINGENE: int = 3
    MINNET: int = 3
    MAXGENE: int = 50
    NEDGES_LOAD: int = 1000000
    MIN_DENSITY: float = 0.6
    
    RESUME_LASTRUN: int = False
    
    LOAD_UNWEIGHTED: bool = False
    RESUME: bool = False
    EXCLUDE_EDGES: bool = True
    LEVEL = 3
    
    InitXYBy: _InitXYBy = _InitXYBy
    MaskStrategyName: _MaskStrategyName = _MaskStrategyName
    OverlapPattern: _OverlapPattern = _OverlapPattern
    Path: _Path = _Path
