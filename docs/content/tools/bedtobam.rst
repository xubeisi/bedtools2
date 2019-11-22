.. _bedtobam:

###############
*bedtobam*
###############
**bedToBam** converts features in a feature file to BAM format. This is useful as an efficient means of
storing large genome annotations in a compact, indexed format for visualization purposes.

==========================================================================
Usage and option summary
==========================================================================
Usage:

::

  bedToBam [OPTIONS] -i <BED/GFF/VCF> -g <GENOME> > <BAM>
  
===========================      ===============================================================================================================================================================================================================
 Option                           Description
===========================      ===============================================================================================================================================================================================================
**-mapq**				         Set a mapping quality (SAM MAPQ field) value for all BED entries. *Default: 255*			 
**-ubam**					     Write uncompressed BAM output. The default is write compressed BAM output.
**-bed12**                       Indicate that the input BED file is in BED12 (a.k.a "blocked" BED) format. In this case, bedToBam will convert blocked BED features (e.g., gene annotations) into "spliced" BAM alignments by creating an appropriate CIGAR string.
**-seq**                         Took sequence from last column of BED6 and write to bam
===========================      ===============================================================================================================================================================================================================




==========================================================================
Default behavior
==========================================================================
The default behavior is to assume that the input file is in unblocked format. For example:

::

  head -5 rmsk.hg18.chr21.bed
  chr21 9719768  9721892  ALR/Alpha  1004  +
  chr21 9721905  9725582  ALR/Alpha  1010  +
  chr21 9725582  9725977  L1PA3 3288 +
  chr21 9726021  9729309  ALR/Alpha  1051  +
  chr21 9729320  9729809  L1PA3 3897 -

  bedToBam -i rmsk.hg18.chr21.bed -g human.hg18.genome > rmsk.hg18.chr21.bam

  samtools view rmsk.hg18.chr21.bam | head -5
  ALR/Alpha  0   chr21 9719769  255  2124M *  0  0  *  *
  ALR/Alpha  0   chr21 9721906  255  3677M *  0  0  *  *
  L1PA3      0   chr21 9725583  255  395M  *  0  0  *  *
  ALR/Alpha  0   chr21 9726022  255  3288M *  0  0  *  *
  L1PA3      16  chr21 9729321  255  489M  *  0  0  *  *
 

==========================================================================
``-seq`` Take BED6 format followed by sequence and write to bam
==========================================================================
Add sequence to the default bed format.

.. code-block:: bash

  bedtools bamtobed -seq -i reads.bam | head -2 > reads.bed

  cat reads.bed
  chr20 62580871 62580972 ATAC/1 255 + CTGCAAGAGTCACCCCCGGGGCCACCCCTCCCAGCAGGTGGTGATGGATGGCCCGCAGCTGTGCACAGTGGGGCAGTCCTGCTTAGGTTCAGCAGCAGGTT
  chr20 62581438 62581539 ATAC/2 255 - TTCTGCTGGGAGTGATGCCCATGCAGCGGACCGGTCACAAGCAGGCCAGGACGATCTGCCAGAAGCCCGCCTCACCGCAGGCCTGTGACGGCGTCAGGCTG

  bedToBam -i reads.bed -seq -g human.hg18.genome > reads.withseq.bam
  bedToBam -i reads.bed -g human.hg18.genome > reads.noseq.bam

  samtools view reads.withseq.bam
  ATAC/1 0 chr20 62580872 255 101M * 0 0 CTGCAAGAGTCACCCCCGGGGCCACCCCTCCCAGCAGGTGGTGATGGATGGCCCGCAGCTGTGCACAGTGGGGCAGTCCTGCTTAGGTTCAGCAGCAGGTT JJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJ
  ATAC/2 16 chr20 62581439 255 101M * 0 0 TTCTGCTGGGAGTGATGCCCATGCAGCGGACCGGTCACAAGCAGGCCAGGACGATCTGCCAGAAGCCCGCCTCACCGCAGGCCTGTGACGGCGTCAGGCTG JJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJ

  samtools view reads.noseq.bam
  ATAC/1 0 chr20 62580872 255 101M * 0 0 * *
  ATAC/2 16 chr20 62581439 255 101M * 0 0 * *

==========================================================================
Creating "spliced" BAM entries from "blocked" BED features
==========================================================================
Optionally, **bedToBam** will create spliced BAM entries from "blocked" BED features by using the
-bed12 option. This will create CIGAR strings in the BAM output that will be displayed as "spliced"
alignments. The image illustrates this behavior, as the top track is a BAM representation (using
bedToBam) of a BED file of UCSC genes.

For example:

::

  bedToBam -i knownGene.hg18.chr21.bed -g human.hg18.genome -bed12 > knownGene.bam
  
  samtools view knownGene.bam | head -2
  uc002yip.1  16   chr21 9928614   2                       5                        5
  
  298M1784N71M1411N93M3963N80M1927N106M3608N81M1769N62M11856N89M98N82M816N61M6910N65M
  738N64M146N100M1647N120M6478N162M1485N51M6777N60M9274N54M880N54M1229N54M2377N54M112
  68N58M2666N109M2885N158M     *   0  0  *  *
  uc002yiq.1  16   chr21 9928614   2                       5                        5
  
  298M1784N71M1411N93M3963N80M1927N106M3608N81M1769N62M11856N89M98N82M816N61M6910N65M
  738N64M146N100M1647N120M6478N162M1485N51M6777N60M10208N54M1229N54M2377N54M11268N58M
  2666N109M2885N158M       *   0   0  *  *


