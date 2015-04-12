package driver;

import java.util.HashSet;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Extractor {
	
	private String data;
	private String regex;
	private Set<String> coauthors = new HashSet<String>();
	
	//Basic constructor that creates an extractor object and scraps all
	//the information it needs from the Google Scholar page.
	public Extractor(String fileName) {
		try {
			this.data = HTMLExtractor.getRawData(fileName);
			this.data = this.data.replaceAll("&#39;", "'");
			this.data = this.data.replaceAll("&#8208", "-");
		} catch (Exception e) {
			System.out.println("malformed URL or cannot open connection to "
			          + "given URL");
		}
	}
	
	//Extract's the scholar's name
	public String extractName() {
	      String regex = "<span id=\"cit-name-display\" "
	              		+ "class=\"cit-in-place-nohover\">(.*?)</span>";
	      Pattern patternObject = Pattern.compile(regex);
	      Matcher matcherObject = patternObject.matcher(this.data);
	      while (matcherObject.find()) {
	    	  return matcherObject.group(1);
	      }
		return null;
	}
	
	////Extract's the scholar's total citations
	public String extractCitations() {
		regex = "Citations</a></td><td class=\"cit-borderleft"+
								" cit-data\">(.*?)</td>";
		Pattern patternObject = Pattern.compile(regex);
		Matcher matcherObject = patternObject.matcher(this.data);
		while (matcherObject.find()) {
			return matcherObject.group(1);
	      }
		return null;
	}
	
	//Extract's the scholar's i10-index entries since 2009
	public String extracti10index(){
		regex = "i10-index</a></td><td class=\"cit-borderleft" + 
								" cit-data\">(.*?)</td><td class=\"cit-"+
								"borderleft cit-data\">(.*?)</td>";
		Pattern patternObject = Pattern.compile(regex);
		Matcher matcherObject = patternObject.matcher(this.data);
		while (matcherObject.find()) {
			return matcherObject.group(2);
		}
		return null;
	}

	//Extract's the scholar's three most recent publications
	public String[] extract3Publications() {
		String[] pubs = new String[3];
		int i = 0;
		regex = "class=\"cit-dark-large-link\">(.*?)</a>";
		Pattern patternObject = Pattern.compile(regex);
		Matcher matcherObject = patternObject.matcher(this.data);
		while (matcherObject.find() && i < 3) {
			pubs[i] = matcherObject.group(1);
			i += 1;
		}
		for (String str : pubs) {
			if (str != null) {}
			else {
				str = "There are no further publications by this authot.";
			}
		}
		return pubs;
	}
	
	//Extracts the scholar's citations on their five most recent articles
	public String extractPaperCitations() {
		int i = 0;
		int sum = 0;
		regex = "<td id=\"col-citedby\"><a class=\"cit-dark-link\" "
				+ "href=\".*?\">(.*?)</a></td>";
		Pattern patternObject = Pattern.compile(regex);
		Matcher matcherObject = patternObject.matcher(this.data);
		while (matcherObject.find() && i < 5) {
			sum += Integer.parseInt(matcherObject.group(1));
			i += 1;
		}
		return Integer.toString(sum);
	}
	
	//Extracts the scholar's list of coauthors from the sidebar.
	public Set<String> extractCoauthors() {
		regex = "<a class=\"cit-dark-link\" href=\".*?\" "
				+ "title=\"(.*?)\">\\1</a>";
		Pattern patternObject = Pattern.compile(regex);
		Matcher matcherObject = patternObject.matcher(this.data);
		while (matcherObject.find()) {
			coauthors.add(matcherObject.group(1));
		}
		return coauthors;
	}
	
	//Extracts the number of coauthors this scholar has.
	public String extractCoauthorAmount() {
		this.extractCoauthors();
		return Integer.toString(coauthors.size());
	}
}
