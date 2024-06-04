import sys
from pathlib import PosixPath, Path

sys.path.insert(0, str(Path(__file__).parent.parent))


import os
import collections
from typing import Optional, Literal
import warnings
from colorama import Fore
import mmap

from nettensorpat.Default import Default
from nettensorpat.wrapper import Validation


class Dataset:
    datasetList: collections.deque[str] = collections.deque()

    def __init__(self, dsPaths: list[str | PosixPath] = None) -> None:
        self.datasetList.extend(map(str, dsPaths) if dsPaths else [])

    @classmethod
    def loadPaths(
        self, dsPath: str | PosixPath, ext: str = Default.Path.DATAFILE_SUFFIX
    ) -> None:
        if isinstance(dsPath, str):
            dsPath = PosixPath(dsPath)

        Validation.validatePath(dsPath, "dsPath", msgType="Error")

        dsPath = str(dsPath.resolve())

        self.datasetList.extend(
            filter(
                lambda x: x.endswith(ext),
                map(
                    lambda x: str(x),
                    filter(
                        lambda x: x.is_file(),
                        map(
                            lambda x: PosixPath(x),
                            os.scandir(dsPath),
                        ),
                    ),
                )
            )
        )
        return self

    @classmethod
    def loadPathsFromFile(
        self,
        dsListFile: str | PosixPath,
        ext: Optional[str] = Default.Path.DATAFILE_SUFFIX,
        dsDir: Optional[str | PosixPath] = None,
        delimiter: str = "\t",
        warn: bool = True,
    ) -> None:
        """Load dataset paths from a list file with the directory of the list file containing the datasets or a user specified directory.

        Args:
            dsListFile (str | PosixPath): Path to the list file containing dataset paths.
            ext (str, optional): Extension of the dataset files. Defaults to Default.Path.DATAFILE_SUFFIX.
            dsDir (str | PosixPath, optional): Directory containing the dataset files. Defaults to None.
            delimiter (str, optional): Delimiter of the list file. Defaults to "\\t".
            warn (bool, optional): Whether to show warnings. Defaults to True.
        """
        warnings.filterwarnings("default")
        if not warn:
            warnings.filterwarnings("ignore")

        if isinstance(dsListFile, str):
            dsListFile = PosixPath(dsListFile)

        dsListFile = dsListFile.resolve()

        if not dsDir:
            dsDir = dsListFile.parent
        else:
            if isinstance(dsDir, str):
                dsDir = PosixPath(dsDir)
            dsDir = dsDir.resolve()

        Validation.validatePath(dsListFile, "dsListFile", msgType="Error")
        Validation.validatePath(dsDir, "dsDir", msgType="Error")

        with open(dsListFile, "r") as f:
            while line := f.readline():
                dsPath = dsDir / f"{line.strip().split(delimiter)[0]}.{ext}"
                try:
                    Validation.validatePath(dsPath, "dsPath", warn=False)
                    self.datasetList.append(dsPath)
                except FileNotFoundError:
                    warnings.warn(
                        f"{Fore.RED}Error:{Fore.RESET} Dataset file {Fore.CYAN}{dsPath}{Fore.RESET} not found. Skipping",
                        UserWarning,
                    )
                    continue
        return self

    @staticmethod
    def convertFromAdjacencyMatrix(
        dsPath: str | PosixPath,
        saveDir: Optional[str | PosixPath] = None,
        saveExt: Optional[str] = Default.Path.DATAFILE_SUFFIX,
        delimiter: str = " ",
        delimiterOutput: str = "\t",
        bufferSize: Optional[int] = None,
        warn: bool = True,
    ) -> None:
        """Converts an adjacency matrix to a dataset file.

        Args:
            dsPath (str | PosixPath): Path to the adjacency matrix file.
            saveDir (Optional[str  |  PosixPath], optional): Directory to save the dataset file. Defaults to None.
            saveExt (Optional[str], optional): Extension of the dataset file. Defaults to Default.Path.DATAFILE_SUFFIX.
            delimiter (str, optional): Delimiter of the adjacency matrix file. Defaults to "\\t".
        """

        if isinstance(dsPath, str):
            dsPath = PosixPath(dsPath)
        dsPath = dsPath.resolve()
        Validation.validatePath(dsPath, "dsPath", msgType="Error")

        if saveDir:
            if isinstance(saveDir, str):
                saveDir = PosixPath(saveDir)
            saveDir = saveDir.resolve()
            Validation.validatePath(saveDir, "saveDir", msgType="Error")
        else:
            saveDir = dsPath.parent
        
        if os.path.exists(f"{saveDir / ''.join(dsPath.name.split('.')[:-1])}.{saveExt}") and os.path.isfile(f"{saveDir / ''.join(dsPath.name.split('.')[:-1])}.{saveExt}"):
            os.remove(f"{saveDir / ''.join(dsPath.name.split('.')[:-1])}.{saveExt}")

        if not bufferSize:
            # Read a line at a time
            with open(dsPath, "r", buffering=1) as f:
                for row, line in enumerate(f):
                    offset = row + 1
                                    
                    line = line.replace(delimiter, "").strip()[offset:]
                    
                    if len(line) == 0:
                        break

                    edgeList = [
                        f"{row}{delimiterOutput}{col+offset}{delimiterOutput}{val}" for col, val in enumerate(line) if val == "1"
                    ]
                    
                    with open(f"{saveDir / ''.join(dsPath.name.split('.')[:-1])}.{saveExt}", "a+") as f:
                        f.write("\n".join(edgeList) + "\n")
        else:
            # Slower method but uses less memory
            row = 0
            col = 0
            
            with open(dsPath, "rb") as f:
                while True:
                    try:
                        buf = f.read(bufferSize)
                    except ValueError:
                        break
                    if not buf:
                        break
                    buf = buf.decode("utf-8").replace(delimiter, "")
                    for i in range(len(buf)):
                        if buf[i] == "\n":
                            row += 1
                            col = 0
                        else:
                            if buf[i] == "1":
                                with open(f"{saveDir / ''.join(dsPath.name.split('.')[:-1])}.{saveExt}", "a+") as f:
                                    f.write(f"{row}{delimiterOutput}{col}{delimiterOutput}1\n")
                            col += 1
        
        if warn:
            print(f"{Fore.GREEN}Success:{Fore.RESET} Converted adjacency matrix to dataset file @ {Fore.CYAN}{saveDir / ''.join(dsPath.name.split('.')[:-1])}.{saveExt}{Fore.RESET}")

    @staticmethod
    def fileType(
        dsPath: str | PosixPath,
    ) -> Literal["adj", "edge"]:
        COL_EDGE_FILE = 3
        
        if isinstance(dsPath, str):
            dsPath = PosixPath(dsPath)
        dsPath = dsPath.resolve()
        Validation.validatePath(dsPath, "dsPath", msgType="Error")
        
        col = 0
        row = 0
        maxVal = -1
        delimiter = None
        
        with open(dsPath, "r+") as f:
            buf = mmap.mmap(f.fileno(), 0)
            readline = buf.readline
            line = readline().decode("utf-8").strip()
            
            if row == 0:
                # Get the number of columns
                delimiters = [
                    " ",
                    "\t",
                    ",",
                    ";",
                    ":"
                ]
                for deli in delimiters:
                    if deli in line:
                        delimiter = deli
                        col = len(line.split(deli))
                        break
                if col == 0:
                    raise ValueError(f"{Fore.RED}Error:{Fore.RESET} Unable to determine delimiter in file {Fore.CYAN}{dsPath}{Fore.RESET}.")
                if col > COL_EDGE_FILE:
                    return "adj"
            elif row > col:
                return "edge"
            
            [
                maxVal := max(maxVal, int(val))
                for val in line.split(delimiter)
                if val.isdigit()
            ]
            
            if maxVal > 1:
                return "adj"
            while readline():
                row += 1
        
        return "adj"
