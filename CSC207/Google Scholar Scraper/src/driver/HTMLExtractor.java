package driver;

import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;
import java.net.URL;

public class HTMLExtractor {
	
	//This grabs an HTML file and reduces it to a string.
	public static String getRawData(String fileName) throws Exception {
	    // create object to store html source text as it is being collected
	    StringBuilder html = new StringBuilder();
	    // open connection to given url
	    URL url = new File(fileName).toURI().toURL();
	    // create BufferedReader to buffer the given url's HTML source
	    BufferedReader htmlbr =
	        new BufferedReader(new InputStreamReader(url.openStream()));
	    String line;
	    // read each line of HTML code and store in StringBuilder
	    while ((line = htmlbr.readLine()) != null) {
	      html.append(line);
	    }
	    htmlbr.close();
	    // convert StringBuilder into a String and return it
	    return html.toString();
	  }

}
