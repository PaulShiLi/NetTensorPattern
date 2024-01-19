import nettensorpat


"""
Level 1 (Default): Output .PATTERN
Level 2: Output .PATTERN .LOG
Level 3: Output .PATTERN .LOG .DENSITIES
"""

# Remove seednode if global

success = nettensorpat.NetTensorPat.frequentClustering(
    geneTotal=50,
    maxNode=50,
    seedNode=0,
    minNode=4,
    maxPattern=2,
    nIteration=3,
    minNetwork=4,
    # minDensity=0.4,
    networkFileSuffix=".sig",
    networkListFile="./test_data/smallScale50x20/datasets/selectedDatasets.list",
    networksPath="./test_data/smallScale50x20/datasets",
    resultsPath="./test_data/runSmallScale50x20/results/",
    level=2,
    local=False,
    resume=False,
    overlapPattern="PATTERN_WITH_BOTH",
    mute=True
)

print(f"Code ran {'successfully' if success else 'unsuccessfully'}.")
