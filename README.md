# 🧬 Network Tensor Pattern

A python package implementation to the C codebase of the paper [A Local Tensor Clustering Algorithm to Annotate Uncharacterized Genes with Many Biological Networks](https://publications.waset.org/abstracts/155115/a-local-tensor-clustering-algorithm-to-annotate-uncharacterized-genes-with-many-biological-networks) with the addition of a second method of Global Tensor Clustering.

---

# 🔔 Requirements

⚙️ OS Supported:

| Windows | 🟢 |
| --- | --- |
| Linux | 🟢 |
| MacOS | 🟢 |

⚠️ Requires Python ≤ 3.11 due to setuptools being deprecated after 3.12

- Consider installing a python environment manager i.e. Anaconda/Miniconda to install Python ≤ 3.11

⚠️ GCC is required in order to compile the C source files and headers into a C Python Extension.

💻 Windows Only:

- Consider [installing MingW via Sourceforge](https://sourceforge.net/projects/mingw/files/Installer/mingw-get-setup.exe/download) (as its website is down as of March 2021)

---

# 🏃 Package Compilation

To compile the C headers and source code:

```bash
python setup.py pnt --compile
```

This should generate a file with our C Python Extensions called `Tensor_Python.so` under  `./src/patnetstensor`

To install the python package:

```bash
pip3 install .
```

---