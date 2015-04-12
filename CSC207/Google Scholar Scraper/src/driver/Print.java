package driver;

import java.io.FileNotFoundException;
import java.io.PrintWriter;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class Print {
	
	private String strToPrint = "";
	private Set<String> coauthors = new HashSet<String>();
	private final String dashes = "------------------------------------"
			+ "-----------------------------------";
	private String charset = "UTF-8";
	
	public Print() {
		strToPrint += dashes;
	}
	
	//This creates a portion of the output dedicated to a particular scholar.
	//All necessary formatting is done as the method is being carried out.
	public String createScholar(Extractor ex) {
		strToPrint += "\n1. Name of Author:\n\t" + ex.extractName();
		strToPrint += "\n2. Number of All Citations:\n\t" 
						+ ex.extractCitations();
		strToPrint += "\n3. Number of i10-index after 2009:\n\t" 
						+ ex.extracti10index();
		strToPrint += "\n4. Title of the first 3 publications:\n\t";
		String[] pubs = ex.extract3Publications();
		int i = 1;
		for (String str : pubs) {
			strToPrint += i + "-\t" + str + "\n";
			if (i < 3) {
				strToPrint += "\t";
			}
			i += 1;
		}
		strToPrint += "5. Total paper citation (first 5 papers):\n\t" 
						+ ex.extractPaperCitations();
		strToPrint += "\n6. Total Co-Authors:\n\t" + ex.extractCoauthorAmount();
		strToPrint += "\n" + dashes;
		coauthors.addAll(ex.extractCoauthors());
		return strToPrint;
	}
	
	//Puts the finishes touches on printing by adding a full coauthor list.
	private String finalizePrinting() {
		strToPrint += "\n7. Co-Author list sorted (Total: " + 
					(coauthors.size() - 1) + "):\n";
		List<String> newList = new ArrayList<String>();
		newList.addAll(coauthors);
		Collections.sort(newList);
		for (String name : newList) {
			strToPrint += name + "\n";
		}
		return strToPrint;
	}
	
	//Used to write the output to a named file using a PrintWriter.
	public void writeToFile(String fileName) 
			throws FileNotFoundException, UnsupportedEncodingException {
		finalizePrinting();
		if (!fileName.endsWith(".txt")) {
			fileName += ".txt";
		}
		PrintWriter writer = new PrintWriter(fileName, charset);
		writer.print(strToPrint);
		writer.close();
	}
	
	//Used to print the output straight to the console.
	public String writeToConsole() {
		System.out.println(finalizePrinting());
		return strToPrint;
	}

}
