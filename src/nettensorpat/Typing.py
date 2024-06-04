from typing import TypedDict, Literal, Optional, Annotated
from dataclasses import dataclass, field
from nettensorpat.Default import Default
from colorama import Fore


# Implement TypedDict for Default
class Status(TypedDict):
    status: bool
    error: Optional[Literal["KeyError", "TypeError", "ValueError", "FileNotFoundError", "RelativePathError"]]
    key: Optional[str]
    msg: Optional[str]


@dataclass
class ValueRange:
    a: tuple[Literal[">", ">="], float | int] | None
    b: tuple[Literal["<", "<="], float | int] | None = field(default=None)

    def validateVal(self, val: float | int, key: any = None) -> Status:
        status = True
        errorMsg = f"{Fore.RED}Warning:{Fore.RESET} Value for {Fore.CYAN}{key}{Fore.RESET} must be"

        min = None
        max = None

        if self.a:
            if ">" in self.a or ">=" in self.a:
                min = self.a
            else:
                max = self.a
        if self.b:
            if "<" in self.b or "<=" in self.b:
                max = self.b
            else:
                min = self.b

        if min:
            if min[0] == ">":
                if val <= min[1]:
                    errorMsg += f" greater than {Fore.YELLOW}{min[1]}{Fore.RESET}"
                    status = False
            elif min[0] == ">=":
                if val < min[1]:
                    errorMsg += f" greater than or equal to {Fore.YELLOW}{min[1]}{Fore.RESET}"
                    status = False
        if max:
            if max[0] == "<":
                if val >= max[1]:
                    if not status:
                        errorMsg += " and"
                    else:
                        status = False
                    errorMsg += f" less than {Fore.YELLOW}{max[1]}{Fore.RESET}"
            elif max[0] == "<=":
                if val > max[1]:
                    if not status:
                        errorMsg += " and"
                    else:
                        status = False
                    errorMsg += (
                        f" less than or equal to {Fore.YELLOW}{max[1]}{Fore.RESET}"
                    )

        return {
            "status": status,
            "error": "ValueError" if not status else None,
            "key": key if not status else None,
            "msg": errorMsg if not status else None,
        }


class ConfigDict(TypedDict):
    maxNode: int
    mute: bool
    local: bool
    seedNode: int
    maxPattern: int
    nIteration: int
    nStage: int
    minNode: Annotated[int, ValueRange((">=", Default.MINGENE))]
    minNetwork: Annotated[int, ValueRange((">=", 0), (">", Default.MINNET))]
    minDensity: Annotated[float, ValueRange((">=", 0), ("<", 1))]
    maskStrategy: Literal["EDGES_PATTERN", "EDGES_ALLNETS", "GENES"]
    overlapPattern: Literal[
        "PATTERN_WITH_NONZEROS_XY",
        "PATTERN_WITH_MORE_NETS",
        "PATTERN_WITH_MORE_GENES",
        "PATTERN_WITH_BOTH",
    ]
    nEdgesLoad: int
    loadUnweighted: bool
    resume: bool
    excludeEdges: bool
    resultFilePrefix: Optional[str]
    networkFileSuffix: Optional[str]
    networksPath: Optional[str]
    resultsPath: Optional[str]
    level: Literal[1, 2, 3]

@dataclass
class Config:
    maxNode: int = field(default=Default.MAXGENE)
    mute: bool = field(default=False)
    local: bool = field(default=False)
    seedNode: int = field(default=Default.InitXYBy.DEFAULT)
    maxPattern: int = field(default=Default.MAXPATTERN)
    nIteration: int = field(default=Default.NITERATION)
    nStage: int = field(default=Default.NSTAGE)
    minNode: int = field(default=Default.MINGENE)
    minNetwork: int = field(default=Default.MINNET)
    minDensity: float = field(default=Default.MIN_DENSITY)
    maskStrategy: Literal["EDGES_PATTERN", "EDGES_ALLNETS", "GENES"] = (
        Default.MaskStrategyName.DEFAULT
    )
    overlapPattern: Literal[
        "PATTERN_WITH_NONZEROS_XY",
        "PATTERN_WITH_MORE_NETS",
        "PATTERN_WITH_MORE_GENES",
        "PATTERN_WITH_BOTH",
    ] = field(default=Default.OverlapPattern.DEFAULT)
    nEdgesLoad: int = field(default=Default.NEDGES_LOAD)
    loadUnweighted: bool = field(default=Default.LOAD_UNWEIGHTED)
    resume: bool = field(default=Default.RESUME)
    excludeEdges: bool = field(default=Default.EXCLUDE_EDGES)
    resultFilePrefix: str = field(default=Default.Path.RESULTFILE_PREFIX)
    networkFileSuffix: str = field(default=Default.Path.DATAFILE_SUFFIX)
    networksPath: str = field(default=Default.Path.NETWORKS_FOLDER)
    resultsPath: str = field(default=Default.Path.RESULT)
    level: Literal[1, 2, 3] = field(default=Default.LEVEL)
