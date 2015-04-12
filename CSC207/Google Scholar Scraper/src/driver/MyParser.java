package driver;
// **********************************************************
// Assignment3:
// CDF user_name: g3dakouz
//
// Author: Daniil Kouznetsov
//
//
// Honor Code: I pledge that this program represents my own
// program code and that I have coded on my own. I received
// help from no one in designing and debugging my program.
// *********************************************************
import java.io.FileNotFoundException;
import java.io.UnsupportedEncodingException;

public class MyParser {
	
	static Print printer = new Print();

  /**
   * This Java class will scrap a Google scholar page and return a number of
   * relevant information about the author.
   * 
   * @param args URLS separated by commas, output file name (optional)
   */
  public static void main(String[] args) {
	  String URLS[] = args[0].split(",");
	  for (String url : URLS) {
	      printer.createScholar(new Extractor(url));
	  }
	  if (args.length == 2 && args[1] != null) {
		  String fileName = args[1];
		  try {
			printer.writeToFile(fileName);
		} catch (FileNotFoundException | UnsupportedEncodingException e) {
			System.err.println("Unable to write file as specified.");
		}
	  }
	  else {
		  printer.writeToConsole();
	  }
  }
}
