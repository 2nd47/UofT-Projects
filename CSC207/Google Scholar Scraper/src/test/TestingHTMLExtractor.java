package test;

import static org.junit.Assert.*;

import org.junit.Before;
import org.junit.Test;

import driver.*;

public class TestingHTMLExtractor {
	
	//A mock HTML file was created and is being tested to see if basic
	//raw data extraction is being carried out properly.
	
	String data;
	
	@Before
	public void setUp() throws Exception {
		data = HTMLExtractor.getRawData("testHTML.html");
	}

	@Test
	public void testGetRawData() {
		assertTrue(data.equals("<html><b>this is raw html data</b></html>"));
	}

}
