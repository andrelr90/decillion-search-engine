# Decillion indexing module:
This module implements an inverted list index on secondary memory. It also includes a notebook for calculating the pageranks of a given collection. The pagerank notebook is a contribution of <b>Celso França</b> (see pagenranks_generator.py).

## Compiling and running:
Use the ```make``` command to compile the module and the ```make run``` command to execute it. Executing this module requires a collection on the jsonline format to be placed on its root folder. See the crawling module for an efficient way of horizontally collecting pages across the web and generating a collection already in this format. The output files of this module will be generated on the <i>inverted_lists</i> folder and will be needed for executing the searching module.
