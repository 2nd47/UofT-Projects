package test;

import static org.junit.Assert.*;

import org.junit.Before;
import org.junit.Test;

import driver.*;

public class TestingExtractor {
	
	//All relevant features of the Extract class are carried out against the
	//two sample Google Scholar HTML files supplied with the Assignment3 files
	
	Extractor ex1;
	Extractor ex2;
	String file1;
	String file2;
	
	@Before
	public void setUp() {
		file1 = "sample1.html";
		file2 = "sample2.html";
		ex1 = new Extractor(file1);
		ex2 = new Extractor(file2);
	}

	@Test
	public void testExtractName() {
		assertTrue(ex1.extractName().equals("Ola Spjuth"));
		assertTrue(ex2.extractName().equals("Yan Xu"));
	}
	
	@Test
	public void testExtractCitations() {
		assertTrue(ex1.extractCitations().equals("437"));
		assertTrue(ex2.extractCitations().equals("263"));
	}
	
	@Test
	public void testExtracti10index() {
		assertTrue(ex1.extracti10index().equals("12"));
		assertTrue(ex2.extracti10index().equals("9"));
	}
	
	@Test
	public void testExtract3Publications() {
		assertTrue(ex1.extract3Publications()[0].equals("Bioclipse: an "
				+ "open source workbench for chemo-and bioinformatics"));
		assertTrue(ex1.extract3Publications()[1].equals("The LCB "
				+ "data warehouse"));
		assertTrue(ex1.extract3Publications()[2].equals("XMPP for cloud "
				+ "computing in bioinformatics supporting "
				+ "discovery and invocation of asynchronous web services"));
		assertTrue(ex2.extract3Publications()[0].equals("Face-tracking as an "
				+ "augmented input in video games: "
				+ "enhancing presence, role-playing and control"));
		assertTrue(ex2.extract3Publications()[1].equals("Art of defense: a "
				+ "collaborative handheld augmented reality board game"));
		assertTrue(ex2.extract3Publications()[2].equals("Sociable killers: "
				+ "understanding social relationships in an online "
				+ "first-person shooter game"));
	}
	
	@Test
	public void testExtractPaperCitations() {
		assertTrue(ex1.extractPaperCitations().equals("239"));
		assertTrue(ex2.extractPaperCitations().equals("158"));
	}
	
	@Test
	public void testExtractCoauthorAmount() {
		assertTrue(ex1.extractCoauthorAmount().equals("15"));
		assertTrue(ex2.extractCoauthorAmount().equals("14"));
	}
}
