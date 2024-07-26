import sys
from pathlib import PosixPath, Path

sys.path.insert(0, str(Path(__file__).parent.parent))


import os
import collections
from typing import Optional, Literal, Union
import warnings
from colorama import Fore
import mmap

from nettensorpat.Default import Default
from nettensorpat.wrapper import Validation


class Dataset:
    datasetList: collections.deque[str] = collections.deque()
    dsPath: str

    def __init__(self, dsPaths: list[str | PosixPath] = None) -> None:
        self.datasetList = collections.deque()
        if dsPaths:
            self.datasetList.extend(map(str, dsPaths))

    @classmethod
    def loadPaths(
        cls, dsPath: str | PosixPath, ext: str = Default.Path.DATAFILE_SUFFIX
    ) -> 'Dataset':
        dataset = cls()
        if isinstance(dsPath, str):
            dsPath = PosixPath(dsPath)

        Validation.validatePath(dsPath, "dsPath", msgType="Error")

        dataset.dsPath = dsPath

        dsPath = str(dsPath.resolve())

        dataset.datasetList.extend(
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
        
        # Sort the dataset list and sort via numbers if in format of name_number.ext
        dataset.datasetList = sorted(dataset.datasetList, key=lambda x: int(x.split("/")[-1].split("_")[-1].split(".")[0]) if "_" in x else x)
        
        return dataset

    @classmethod
    def loadPathsFromFile(
        cls,
        dsListFile: str | PosixPath,
        ext: Optional[str] = Default.Path.DATAFILE_SUFFIX,
        dsDir: Optional[str | PosixPath] = None,
        delimiter: str = "\t",
        warn: bool = True,
    ) -> 'Dataset':
        dataset = cls()
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

        dataset.dsPath = dsDir

        Validation.validatePath(dsListFile, "dsListFile", msgType="Error")
        Validation.validatePath(dsDir, "dsDir", msgType="Error")

        with open(dsListFile, "r") as f:
            while line := f.readline():
                dsPath = dsDir / f"{line.strip().split(delimiter)[0]}.{ext}"
                try:
                    Validation.validatePath(dsPath, "dsPath", warn=False)
                    dataset.datasetList.append(dsPath)
                except FileNotFoundError:
                    warnings.warn(
                        f"{Fore.RED}Error:{Fore.RESET} Dataset file {Fore.CYAN}{dsPath}{Fore.RESET} not found. Skipping",
                        UserWarning,
                    )
                    continue
        return dataset

    @staticmethod
    def loadPathsFromMultipleLists(
        directory: str | PosixPath,
        file_pattern: str = "selectedDatasets_{}.list",
        start: Optional[int] = None,
        end: Optional[int] = None,
        listFileOnly: bool = False,
        delimiter: str = "\t",
    ) -> Union['Dataset', list[str | PosixPath]]:
        """Loads multiple list files based on a pattern and a range.

        Args:
            directory (str | PosixPath): Directory containing the list files.
            file_pattern (str, optional): Pattern of the filename, with a placeholder for the index. Defaults to 'selectedDatasets_{}.list'.
            start (Optional[int], optional): Starting index for the range. Defaults to None.
            end (Optional[int], optional): Ending index for the range. Defaults to None.
            delimiter (str, optional): Delimiter used in the list files. Defaults to "\\t".

        Returns:
            Dataset: An instance of Dataset with the combined dataset list.
        """
        
        if isinstance(directory, str):
            directory = PosixPath(directory)
        directory = directory.resolve()
        
        combined_dataset = Dataset()
        combined_dataset.dsPath = directory

        if start is None and end is None:
            # Load all list files that match the pattern in the directory
            list_files = sorted(directory.glob(file_pattern.replace('{}', '*')), key=lambda x: int(x.stem.split('_')[-1]))
        else:
            # Load list files in the specified range
            list_files = [directory / file_pattern.format(i) for i in range(start, end + 1)]
        
        if listFileOnly:
            return list_files
        
        for list_file in list_files:
            if list_file.exists() and list_file.is_file():
                with open(list_file, "r") as f:
                    print(f"Loading {list_file}...")
                    while line := f.readline():
                        dsPath = directory / f"{line.strip().split(delimiter)[0]}"
                        try:
                            combined_dataset.datasetList.append(dsPath)
                        except FileNotFoundError:
                            warnings.warn(
                                f"{Fore.RED}Error:{Fore.RESET} Dataset file {Fore.CYAN}{dsPath}{Fore.RESET} not found. Skipping",
                                UserWarning,
                            )
                            continue
            else:
                print(f"File {list_file} does not exist or is not a file.")
                
        return combined_dataset

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
        """Determines the type of dataset file.

        Raises:
            ValueError: Unable to determine delimiter in file.

        Returns:
            Literal["adj", "edge"]: Type of dataset file.
        """
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

    def generateList(
        self,
        saveDir: Optional[str | PosixPath] = None,
        delimiter: str = "\t",
        overwrite: bool = False,
        num_parts: Optional[int] = None,
    ) -> None:
        """Generates a list of dataset paths, optionally split into multiple parts.

        Args:
            saveDir (Optional[str | PosixPath], optional): Directory to save the list files. Defaults to None.
            delimiter (str, optional): Delimiter of the list files. Defaults to "\\t".
            overwrite (bool, optional): Whether to overwrite the list files if they already exist. Defaults to False.
            num_parts (Optional[int], optional): Number of parts to split the dataset list into. If None, all datasets are saved in one file.
        """
        
        if isinstance(saveDir, str):
            saveDir = PosixPath(saveDir)
            saveDir = saveDir.resolve()     
        
        if not saveDir:
            saveDir = self.dsPath.parent

        total_files = len(self.datasetList)
        if num_parts is None or num_parts <= 1:
            list_file = saveDir / 'selectedDatasets.list'
            if (list_file.exists() and list_file.is_file() and overwrite) or (not list_file.exists()):
                with open(list_file, "w") as f:
                    print("Generating list file...")
                    f.write("\n".join(
                        [
                            f"{''.join(ds.split(f'{os.sep}')[-1].split('.')[:-1])}{delimiter}{i}" for i, ds in enumerate(self.datasetList)
                        ]
                    ))
            else:
                warnings.warn(
                    f"{Fore.RED}Error:{Fore.RESET} List file {Fore.CYAN}{list_file}{Fore.RESET} already exists. Skipping",
                    UserWarning,
                )
        else:
            batch_size = total_files // num_parts
            for i in range(num_parts):
                start_idx = i * batch_size
                if i == num_parts - 1:  # Last batch takes the remainder
                    end_idx = total_files
                else:
                    end_idx = start_idx + batch_size
                batch = list(self.datasetList)[start_idx:end_idx]
                list_file = saveDir / f'selectedDatasets_{i + 1}.list'
                if (list_file.exists() and list_file.is_file() and overwrite) or (not list_file.exists()):
                    with open(list_file, "w") as f:
                        print(f"Generating list file {list_file}...")
                        f.write("\n".join(
                            [
                                f"{''.join(ds.split(f'{os.sep}')[-1].split('.')[:-1])}{delimiter}{idx}" for idx, ds in enumerate(batch)
                            ]
                        ))
                else:
                    warnings.warn(
                        f"{Fore.RED}Error:{Fore.RESET} List file {Fore.CYAN}{list_file}{Fore.RESET} already exists. Skipping",
                        UserWarning,
                    )
