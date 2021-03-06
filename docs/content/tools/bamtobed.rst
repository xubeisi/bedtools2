.. _bamtobed:

###############
*bamtobed*
###############
``bedtools bamtobed`` is a conversion utility that converts sequence alignments 
in BAM format into BED, BED12, and/or BEDPE records. 

==========================================================================
Usage and option summary
==========================================================================
**Usage**:
::

  bedtools bamtobed [OPTIONS] -i <BAM>

**(or)**:
::

    bamToBed [OPTIONS] -i <BAM>



.. tabularcolumns:: |p{4.5cm}|p{8.5cm}|

=============   ================================================================
Option          Description
=============   ================================================================
**-seq**        Write sequence addtional to bed6
**-bedpe**      Write BAM alignments in BEDPE format. Only one alignment from 
                paired-end reads will be reported. Specifically, it each mate 
                is aligned to the same chromosome, the BAM alignment reported 
                will be the one where the BAM insert size is greater than zero. 
                When the mate alignments are interchromosomal, the 
                lexicographically lower chromosome will be reported first. 
                Lastly, when an end is unmapped, the chromosome and strand will 
                be set to "." and the start and end coordinates will be set 
                to -1. *By default, this is disabled and the output will be 
                reported in BED format*.
**-bedpeseq**   Write sequence addtional to -bedpe
**-mate1**      When writing BEDPE (-bedpe) format,
                always report mate one as the first BEDPE "block".		 
**-bed12**      Write "blocked" BED (a.k.a. BED12) format. This will convert 
                "spliced" BAM alignments (denoted by the "N" CIGAR operation) 
                to BED12. `Forces -split`.
**-split**      Report each portion of a "split" BAM (i.e., having an "N" CIGAR 
                operation) alignment as a distinct BED intervals.
**-splitD**     Report each portion of a "split" BAM while obeying both "N" CIGAR 
                and "D" operation. Forces `-split`.
**-ed**         Use the "edit distance" tag (NM) for the BED score field. 
                Default for BED is to use mapping quality. Default for BEDPE is 
                to use the *minimum* of the two mapping qualities for the pair. 
                When -ed is used with -bedpe, the total edit distance from the 
                two mates is reported.                                            
**-tag**        Use other *numeric* BAM alignment tag for BED score. Default 
                for BED is to use mapping quality. Disallowed with BEDPE output.
**-color**      An R,G,B string for the color used with BED12 format. Default 
                is (255,0,0).
**-cigar**      Add the CIGAR string to the BED entry as a 7th column.
=============   ================================================================


==========================================================================
Default behavior
==========================================================================
By default, each alignment in the BAM file is converted to a 6 column BED. The 
BED "name" field is comprised of the RNAME field in the BAM alignment. If mate 
information is available, the mate (e.g., "/1" or "/2") field will be appended 
to the name.

.. code-block:: bash

  $ bedtools bamtobed -i reads.bam | head -3
  chr7   118970079   118970129   TUPAC_0001:3:1:0:1452#0/1   37   -
  chr7   118965072   118965122   TUPAC_0001:3:1:0:1452#0/2   37   +
  chr11  46769934    46769984    TUPAC_0001:3:1:0:1472#0/1   37   -

==========================================================================
``-seq`` Add sequence to bed
==========================================================================
Add sequence to the default bed format.

.. code-block:: bash

  $ bedtools bamtobed -seq -i reads.bam | head -2
  chr20 62580871 62580972 ATAC/1 255 + CTGCAAGAGTCACCCCCGGGGCCACCCCTCCCAGCAGGTGGTGATGGATGGCCCGCAGCTGTGCACAGTGGGGCAGTCCTGCTTAGGTTCAGCAGCAGGTT
  chr20 62581438 62581539 ATAC/2 255 - TTCTGCTGGGAGTGATGCCCATGCAGCGGACCGGTCACAAGCAGGCCAGGACGATCTGCCAGAAGCCCGCCTCACCGCAGGCCTGTGACGGCGTCAGGCTG

==========================================================================
``-tag`` Set the score field based on BAM tags
==========================================================================
One can override the choice of the BAM `MAPQ` as the result BED record's `score`
field by using the ``-tag`` option.  In the example below, we use the ``-tag``
option to select the BAM edit distance (the `NM` tag) as the score 
column in the resulting BED records.

.. code-block:: bash

  $ bedtools bamtobed -i reads.bam -tag NM | head -3
  chr7   118970079   118970129   TUPAC_0001:3:1:0:1452#0/1   1    -
  chr7   118965072   118965122   TUPAC_0001:3:1:0:1452#0/2   3    +
  chr11  46769934    46769984    TUPAC_0001:3:1:0:1472#0/1   1    -


==========================================================================
``-bedpe`` Set the score field based on BAM tags
==========================================================================
The ``-bedpe`` option converts BAM alignments to BEDPE format, thus allowing
the two ends of a paired-end alignment to be reported on a single text line. 
Specifically, it each mate is aligned to the same chromosome, 
the BAM alignment reported will be the one where the BAM insert size is greater 
than zero. When the mate alignments are interchromosomal, the lexicographically 
lower chromosome will be reported first. Lastly, when an end is unmapped, the 
chromosome and strand will be set to "." and the start and end coordinates will 
be set to -1. 

.. note::

    When using this option, it is required that the BAM 
    file is sorted/grouped by the read name. This allows bamToBed 
    to extract correct alignment coordinates for each end based on 
    their respective CIGAR strings. It also assumes that the 
    alignments for a given pair come in groups of twos. There is 
    not yet a standard method for reporting multiple alignments 
    using BAM. bamToBed will fail if an aligner does not report 
    alignments in pairs.		

.. code-block:: bash
 
  $ bedtools bamtobed -i reads.bam -bedpe | head -3
  chr7   118965072   118965122   chr7   118970079   118970129 TUPAC_0001:3:1:0:1452#0 37     +     -
  chr11  46765606    46765656    chr11  46769934    46769984 TUPAC_0001:3:1:0:1472#0 37     +     -
  chr20  54704674    54704724    chr20  54708987    54709037 TUPAC_0001:3:1:1:1833#0 37     +    

		 
One can easily use samtools and bamToBed together as part of a UNIX pipe. In 
this example, we will only convert properly-paired (``FLAG == 0x2``) reads to 
BED format.

.. code-block:: bash

  $ samtools view -bf 0x2 reads.bam | bedtools bamtobed -i stdin | head
  chr7   118970079   118970129   TUPAC_0001:3:1:0:1452#0/1   37   -
  chr7   118965072   118965122   TUPAC_0001:3:1:0:1452#0/2   37   +
  chr11  46769934    46769984    TUPAC_0001:3:1:0:1472#0/1   37   -
  chr11  46765606    46765656    TUPAC_0001:3:1:0:1472#0/2   37   +
  chr20  54704674    54704724    TUPAC_0001:3:1:1:1833#0/1   37   +
  chr20  54708987    54709037    TUPAC_0001:3:1:1:1833#0/2   37   -
  chrX   9380413     9380463     TUPAC_0001:3:1:1:285#0/1    0    -
  chrX   9375861     9375911     TUPAC_0001:3:1:1:285#0/2    0    +
  chrX   131756978   131757028   TUPAC_0001:3:1:2:523#0/1    37   +
  chrX   131761790   131761840   TUPAC_0001:3:1:2:523#0/2    37   -

==========================================================================
``-bedpeseq`` Add sequences for -bedpe
==========================================================================
The ``-bedpeseq`` option converts BAM alignments to BEDPE format with two addtional
columns as sequences for the first mate and second mate

.. note::

    When using this option, it is required that the BAM 
    file is sorted/grouped by the read name. This allows bamToBed 
    to extract correct alignment coordinates for each end based on 
    their respective CIGAR strings. It also assumes that the 
    alignments for a given pair come in groups of twos. There is 
    not yet a standard method for reporting multiple alignments 
    using BAM. bamToBed will fail if an aligner does not report 
    alignments in pairs.		

.. code-block:: bash
 
  $ bedtools bamtobed -i reads.bam -bedpeseq | head -1
  chr20 62580871 62580972 chr20 62581438 62581539 ATAC 255 + - CTGCAAGAGTCACCCCCGGGGCCACCCCTCCCAGCAGGTGGTGATGGATGGCCCGCAGCTGTGCACAGTGGGGCAGTCCTGCTTAGGTTCAGCAGCAGGTT TTCTGCTGGGAGTGATGCCCATGCAGCGGACCGGTCACAAGCAGGCCAGGACGATCTGCCAGAAGCCCGCCTCACCGCAGGCCTGTGACGGCGTCAGGCTG

==================================================================
``-split`` Creating BED12 features from "spliced" BAM entries. 
==================================================================
``bedtools bamtobed`` will, by default, create a BED6 feature that represents 
the entire span of a spliced/split BAM alignment. However, when using the 
``-split`` command, a BED12 feature is reported where BED blocks will be 
created for each aligned portion of the sequencing read.

::

  Chromosome  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
             
  Exons       ***************                                    **********
  
  BED/BAM A      ^^^^^^^^^^^^....................................^^^^
  
  Result      ===============                                    ====
  
