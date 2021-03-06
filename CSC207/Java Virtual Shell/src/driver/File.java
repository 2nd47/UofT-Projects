/**
 * 
 * File class which represents files whose contents are able to edited and 
 * whose location is always within a parent directory.
 * 
 * contents is a string of characters which represents the contents of a 
 * txt file
 * 
 * @author c3chownl, c4paprak, c3tsafat, g3dakouz
 *
 */


package driver;
import java.net.*;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.io.*;
public class File extends FileSystem {

	private String contents = "";

	// N.B. Creation of new files should be called by Directory addFile()!

	/**
	 * Constructor for a named file with a parent directory (Files NEED a 
	 * parent directory)
	 * 
	 * @param fileName
	 *            name of the new file
	 * @param fileDirectory
	 *            parent directory of the new file
	 */
	public File(String fileName, Directory fileDirectory) {
		// Method stub (named constructor; files should ALWAYS be named)
		super(fileName, fileDirectory);
	}

	/**
	 * Constructor for a named file with a parent directory which also has
	 * contents already within it.
	 * 
	 * @param fileName
	 *            name of the new file
	 * @param fileDirectory
	 *            parent directory of the new file
	 * @param fileContents
	 *            contents of the new file
	 */
	public File(String fileName, Directory fileDirectory, String fileContents) 
	{
		// Method stub (named constructor that can also add contents)
		super(fileName, fileDirectory);
		// fileDirectory.addFile(this);
		contents = fileContents;
	}

	/**
	 * Associate this file with a parent directory.
	 * 
	 * @param parentDirectory
	 *            the new parent directory
	 */
	public void addToParent(Directory parentDirectory) {
		this.parent = parentDirectory;
		parentDirectory.addFile(this);
	}

	/**
	 * Delete this file from its parent directory's list of files and remove
	 * this file from the virtual file system.
	 */
	public void deleteFileSystem() {
		this.parent.removeFile(this);
		super.deleteFileSystem();
	}

	/**
	 * Change the contents of this file by either overwriting or appending a
	 * string to the file's contents
	 * 
	 * @param newString
	 *            the new contents of the file
	 * @param overwrite
	 *            determines whether the old contents are removed
	 */
	public void alterContents(String newString, Boolean overwrite) {
		// Method stub (should be able to overwrite all contents or append)
		if (overwrite) {
			this.contents = newString;
		} else {
			this.contents += newString;
		}
	}

	/**
	 * Checks if this object is equal to another directory object. Used for
	 * testing
	 * 
	 * @param directoryObject
	 *            the directory object to be specified
	 * @return whether or not the two objects are the same
	 */
	public boolean equals(Directory directoryObject) {
		return super.equals(directoryObject);
	}

	/**
	 * Get the contents of the file
	 * 
	 * @return the contents of the file as a string
	 */
	public String getContents() {
		return this.contents;
	}
	
	public void URLReader(String GivenURL) throws IOException {
	        URL urlToGet;
			try {
				urlToGet = new URL(GivenURL);
				BufferedReader in = new BufferedReader(
						new InputStreamReader(urlToGet.openStream()));
				String inputLine;
				while ((inputLine = in.readLine()) != null)
					this.alterContents(inputLine, false);
				in.close();
			} catch (MalformedURLException e) {
				System.out.println("Invalid URL");
			}
	}
	
	public static File createFileFromURL (String GivenURL) {
		List<String> parsedURL = Arrays.asList(GivenURL.trim().split("/"));
		String newFileName = parsedURL.get(parsedURL.size()-1);
		File parsedURLFile = new File(newFileName, Directory.workingDirectory);
		return parsedURLFile;
	}
	
	
	//need to add if path does't exist create file in cwd
	public static void fileRedirection (String output, String outFile, String overWrite) {
		if (Input.checkPath(outFile, true)) {
			Directory currentParent = (Directory) Input.parsePath(outFile)[0];
			if (Input.parsePath(outFile)[1] instanceof Directory) {
				System.out.println("Cannot create a file that is the same name as a directory's sub-directory");
				return;
			}
			File currentFile = (File) Input.parsePath(outFile)[1];
			List <String> pathList = Arrays.asList(outFile.trim().split("/"));
			String fileName = pathList.get(pathList.size()-1);
			if (currentParent.getFiles().contains(currentFile)) {
				if (overWrite.equals(">")) {
					currentFile.alterContents(output, true);
				}
				else {
					currentFile.alterContents(output, false);
				}
			}
			else {
				File newFile = new File(fileName, currentParent, output);
			}
			
		}
	}
	
	/**
	 * Translates the directory's location as an object to a path which is able
	 * to be interpreted by the user.
	 * 
	 * @return the string representation of this object's path
	 */
	public String fileLocation() {
		String dirPath = "";
		dirPath = this.parent.directoryLocation();
		if (!dirPath.equals("/")) {
			dirPath = dirPath + "/";
		}
		dirPath = dirPath + this.getName();
		return dirPath;
	}
	
}