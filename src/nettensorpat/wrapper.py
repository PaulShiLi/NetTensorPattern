import sys
from pathlib import Path, PosixPath

sys.path.insert(0, str(Path(__file__).parent.parent))

import nettensorpat.Tensor_Python as Tensor_Python
from typing import Literal, get_origin, Union, Annotated
from dacite import from_dict
from dataclasses import asdict
from nettensorpat.Typing import ConfigDict, Config, Status, ValueRange
from nettensorpat.Default import Default, Info
import os

# For better UI
import warnings
from colorama import Fore

DEFAULT_CONFIG = Config()


class Validation:
    def validatePath(
        path: str | PosixPath,
        key: str = None,
        warn: bool = True,
        msgType: Literal["warning", "error"] = "warning",
    ) -> Status:
        warnings.filterwarnings("default")

        if not warn:
            warnings.filterwarnings("ignore")

        # Convert PosixPath to string
        if isinstance(path, PosixPath):
            path = str(path)

        if not os.path.exists(os.path.abspath(os.path.join(os.getcwd(), path))):
            raise FileNotFoundError(
                f"{Fore.RED}Error:{Fore.RESET} Path {path} does not exist"
            )

        if path.startswith("."):
            warnings.warn(
                f"\n{Fore.RED}{msgType}:{Fore.RESET} Value for {Fore.CYAN}{key}{Fore.RESET} is a relative path. Recommend to utilize full path.",
            )
            return {
                "error": "RelativePathError",
                "key": key,
                "msg": f"\n{Fore.RED}Warning:{Fore.RESET} Value for {Fore.CYAN}{key}{Fore.RESET} is a relative path. Recommend to utilize full path.",
                "status": False,
            }
        return {"error": None, "key": key, "msg": None, "status": True}

    def validateConfig(
        config: ConfigDict = None,
        warn: bool = True,
        msgType: Literal["warning", "error"] = "warning",
    ) -> Status:
        msgType = msgType.capitalize()
        warnings.filterwarnings("default")

        if not warn:
            warnings.filterwarnings("ignore")

        # General typing, key and value validation
        for k, v in config.items():
            if k in ConfigDict.__dict__["__annotations__"]:
                # Check if value is of the correct type
                if get_origin(ConfigDict.__dict__["__annotations__"][k]) == Literal:
                    if v not in ConfigDict.__dict__["__annotations__"][k].__args__:
                        warnings.warn(
                            f"\n{Fore.RED}{msgType}:{Fore.RESET} Value for {Fore.CYAN}{k}{Fore.RESET} is not a valid {Fore.BLUE}{type(ConfigDict.__dict__['__annotations__'][k].__args__[0]).__name__}{Fore.RESET} value from the following options: {Fore.YELLOW}{(Fore.RESET + ', ' + Fore.YELLOW).join([str(i) for i in ConfigDict.__dict__['__annotations__'][k].__args__])}{Fore.RESET}",
                            SyntaxWarning,
                        )
                        # print(f"Value for `{k}` is not a valid value from the following options: {', '.join([str(i) for i in ConfigDict.__dict__['__annotations__'][k].__args__])}")
                        return {"status": False, "error": "ValueError", "key": k}
                elif get_origin(ConfigDict.__dict__["__annotations__"][k]) == Union:
                    if not any(
                        [
                            isinstance(v, i)
                            for i in ConfigDict.__dict__["__annotations__"][k].__args__
                        ]
                    ):
                        warnings.warn(
                            f"\n{Fore.RED}{msgType}:{Fore.RESET} Value for {Fore.CYAN}{k}{Fore.RESET} is not of the correct type {Fore.BLUE}{(Fore.RESET + ', ' + Fore.BLUE).join([i.__name__ for i in ConfigDict.__dict__['__annotations__'][k].__args__])}{Fore.RESET}",
                            EncodingWarning,
                        )
                        return {"status": False, "error": "TypeError", "key": k}
                elif get_origin(ConfigDict.__dict__["__annotations__"][k]) == Annotated:
                    if not any(
                        [
                            isinstance(v, i)
                            for i in ConfigDict.__dict__["__annotations__"][k].__args__
                        ]
                    ):
                        warnings.warn(
                            f"\n{Fore.RED}{msgType}:{Fore.RESET} Value for {Fore.CYAN}{k}{Fore.RESET} is not of the correct type {Fore.BLUE}{(Fore.RESET + ', ' + Fore.BLUE).join([i.__name__ for i in ConfigDict.__dict__['__annotations__'][k].__args__])}{Fore.RESET}",
                            EncodingWarning,
                        )
                        return {"status": False, "error": "TypeError", "key": k}
                    else:
                        stat: Status = (
                            ConfigDict.__dict__["__annotations__"][k]
                            .__metadata__[0]
                            .validateVal(v, k)
                        )
                        if not stat["status"]:
                            warnings.warn(
                                "\n" + stat["msg"],
                                EncodingWarning,
                            )
                            return stat
                elif not isinstance(v, ConfigDict.__dict__["__annotations__"][k]):
                    warnings.warn(
                        f"\n{Fore.RED}{msgType}:{Fore.RESET} Value for {Fore.CYAN}{k}{Fore.RESET} is not of the correct type {Fore.BLUE}{ConfigDict.__dict__['__annotations__'][k].__name__}{Fore.RESET}",
                        EncodingWarning,
                    )
                    return {"status": False, "error": "TypeError", "key": k}
            else:
                warnings.warn(
                    f"\n{Fore.RED}{msgType}:{Fore.RESET} Key {Fore.CYAN}{k}{Fore.RESET} is not a valid key in the config",
                    SyntaxWarning,
                )
                return {"status": False, "error": "KeyError", "key": k}

        ### Specific Manual Validation

        # Check if directories are valid
        pathKeys = ["networksPath", "resultsPath"]
        for k in pathKeys:
            if k in config:
                try:
                    status: Status = Validation.validatePath(
                        config[k], key=k, warn=warn
                    )
                    if status["error"] == "RelativePathError":
                        return status
                except FileNotFoundError:
                    warnings.warn(
                        f"\n{Fore.RED}{msgType}:{Fore.RESET} {Fore.CYAN}{config[k]}{Fore.RESET} does not exist",
                        SyntaxWarning,
                    )
                    return {"status": False, "error": "FileNotFoundError", "key": k}

        return {"status": True, "error": None, "key": None}

    def resolution(config: ConfigDict, status: Status, warn: bool = True) -> ConfigDict:
        warnings.filterwarnings("default")

        if not warn:
            warnings.filterwarnings("ignore")
        if status["error"] == "KeyError":
            config.pop(status["key"])
            warnings.warn(
                f"\n{Fore.GREEN}Resolution:{Fore.RESET} Key {Fore.CYAN}{status['key']}{Fore.RESET} has been removed from the config",
                SyntaxWarning,
            )
        elif status["error"] == "TypeError":
            config[status["key"]] = DEFAULT_CONFIG.__dict__[status["key"]]
            warnings.warn(
                f"\n{Fore.GREEN}Resolution:{Fore.RESET} Value for {Fore.CYAN}{status['key']}{Fore.RESET} has been set to default value {Fore.YELLOW}{DEFAULT_CONFIG.__dict__[status['key']]}{Fore.RESET}",
                SyntaxWarning,
            )
        elif status["error"] == "ValueError":
            config[status["key"]] = DEFAULT_CONFIG.__dict__[status["key"]]
            warnings.warn(
                f"\n{Fore.GREEN}Resolution:{Fore.RESET} Value for {Fore.CYAN}{status['key']}{Fore.RESET} has been set to default value {Fore.YELLOW}{DEFAULT_CONFIG.__dict__[status['key']]}{Fore.RESET}",
                SyntaxWarning,
            )
        elif status["error"] == "FileNotFoundError":
            config[status["key"]] = str(DEFAULT_CONFIG.__dict__[status["key"]])
            warnings.warn(
                f"\n{Fore.GREEN}Resolution:{Fore.RESET} Value for {Fore.CYAN}{status['key']}{Fore.RESET} has been set to default value {Fore.YELLOW}{DEFAULT_CONFIG.__dict__[status['key']]}{Fore.RESET}",
                SyntaxWarning,
            )
            # Validate if default test data exists and if not, allow FileNotFound error to end the program
            Validation.validatePath(config[status["key"]], key=status["key"], warn=warn)
        elif status["error"] == "RelativePathError":
            config[status["key"]] = os.path.abspath(
                os.path.join(os.getcwd(), config[status["key"]])
            )
            warnings.warn(
                f"\n{Fore.GREEN}Resolution:{Fore.RESET} Value for {Fore.CYAN}{status['key']}{Fore.RESET} has been set to absolute path {Fore.YELLOW}{config[status['key']]}{Fore.RESET}",
                SyntaxWarning,
            )
        return config



class NetTensorPat:
    DEFAULT = Default
    CONFIG: ConfigDict = None

    def __init__(self, config: ConfigDict = None) -> None:
        self.CONFIG = asdict(self._serializeConfig(config))

    def _serializeConfig(self, config: ConfigDict) -> Config:
        return from_dict(Config, config or {})

    def writeConfig(self, config: ConfigDict, warn: bool = True) -> None:
        warnings.filterwarnings("default")

        if not warn:
            warnings.filterwarnings("ignore")

        validated = False
        while not validated:
            status = Validation.validateConfig(config, warn=warn)
            validated = status["status"]

            if not validated:
                config = Validation.resolution(config, status, warn=warn)
        self.CONFIG = asdict(self._serializeConfig(config))

    def frequentClustering(
        self,
        geneTotal: Annotated[int, ValueRange((">=", 0), ("<", DEFAULT_CONFIG.minNode))],
        networkListFile: str,
        config: ConfigDict,
        warn: bool = True,
    ) -> bool:
        self.writeConfig(config, warn=warn)
        Validation.validatePath(networkListFile, warn=False)

        warnings.filterwarnings("default")
        if not warn:
            warnings.filterwarnings("ignore")

        geneTotal_validation = ValueRange(
            (">=", 0)#, ("<", DEFAULT_CONFIG.minNode)
        ).validateVal(geneTotal, "geneTotal")
        # Validate geneTotal
        if not geneTotal_validation["status"]:
            warnings.warn("\n" + geneTotal_validation["msg"], EncodingWarning)
            geneTotal = DEFAULT_CONFIG.maxNode
            warnings.warn(
                f"\n{Fore.GREEN}Resolution:{Fore.RESET} Value for {Fore.CYAN}geneTotal{Fore.RESET} has been set to default value {Fore.YELLOW}{DEFAULT_CONFIG.minNode}{Fore.RESET}",
                SyntaxWarning,
            )

        networkListFile_validation = Validation.validatePath(
            networkListFile, key="networkListFile", warn=warn
        )
        if not networkListFile_validation["status"]:
            if os.path.abspath(os.path.join(os.getcwd(), networkListFile)):
                networkListFile = os.path.abspath(
                    os.path.join(os.getcwd(), networkListFile)
                )
                warnings.warn(
                    f"\n{Fore.GREEN}Resolution:{Fore.RESET} Value for {Fore.CYAN}networkListFile{Fore.RESET} has been set to absolute path {Fore.YELLOW}{networkListFile}{Fore.RESET}",
                    SyntaxWarning,
                )
            else:
                return False

        return Tensor_Python.frequentClustering(
            geneTotal,
            networkListFile,
            self.CONFIG["maxNode"],
            self.CONFIG["mute"],
            self.CONFIG["local"],
            self.CONFIG["seedNode"],
            self.CONFIG["maxPattern"],
            self.CONFIG["nIteration"],
            self.CONFIG["nStage"],
            self.CONFIG["minNode"],
            self.CONFIG["minNetwork"],
            self.CONFIG["minDensity"],
            self.CONFIG["maskStrategy"],
            self.CONFIG["overlapPattern"],
            self.CONFIG["nEdgesLoad"],
            self.CONFIG["loadUnweighted"],
            self.CONFIG["resume"],
            self.CONFIG["excludeEdges"],
            self.CONFIG["resultFilePrefix"],
            self.CONFIG["networkFileSuffix"],
            self.CONFIG["networksPath"],
            self.CONFIG["resultsPath"],
            self.CONFIG["level"],
        )
