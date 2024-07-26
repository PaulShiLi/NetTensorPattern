from nettensorpat import NetTensorPat, Typing, Dataset
import nettensorpat

"""
Level 1 (Default): Output .PATTERN
Level 2: Output .PATTERN .LOG
Level 3: Output .PATTERN .LOG .DENSITIES
"""

ds = Dataset.loadPaths(
    "/u/scratch/w/wenyuan/proj/paper_algorithm_2024.07/dataset"
)
ds.generateList(
    "/u/scratch/w/wenyuan/proj/paper_algorithm_2024.07/dataset",
    overwrite=True
)

netTensor = NetTensorPat()

nettensorConfig: Typing.ConfigDict = {
    "seedNode": 0,
    "maxNode": 50,
    "minNode": 4,
    "maxPattern": 2,
    "nIteration": 3,
    "minNetwork": 4,
    "networkFileSuffix": ".sig",
    "networksPath": "./test/datasets",
    "resultsPath": "./test/results/",    
    "level": 2,
    "resume": False,
    "overlapPattern": "PATTERN_WITH_BOTH",
    "mute": True,
    "local": True
}

# success = netTensor.frequentClustering(
#     geneTotal=50,
#     networkListFile="./test/datasets/selectedDatasets.list",
#     config = nettensorConfig
# )

# print(f"Code ran {'successfully' if success else 'unsuccessfully'}.")


# import timeit
# from tqdm import tqdm

# ds = Dataset.loadPaths("/Users/sub01/Datasets/contracted-nets_ds", ext="cnet")

# # ds = Dataset.loadPathsFromFile(
# #     "./test_data/smallScale50x20/datasets/selectedDatasets.list"
# # )

# print(ds)

# start = timeit.timeit()
# for dsPath in tqdm(ds.datasetList):
#     # fType = ds.fileType(dsPath)
#     ds.convertFromAdjacencyMatrix(dsPath, "/Users/sub01/Datasets/contracted-nets_ds/sig", warn=False)

# print(f"{Fore.LIGHTBLACK_EX}Time taken: {Fore.YELLOW}{timeit.timeit() - start}{Fore.RESET}")
# # print(ds.datasetList)