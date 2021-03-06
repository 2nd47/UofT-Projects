//**********************************************************
// Assignment2:
// CDF user_name1:g3dakouz
// CDF user_name2:c4paprak
// CDF user_name3:c3tsafat
// CDF user_name4:c3chownl
//
// Author1:Daniil Kouznetsov
// Author2:Julianna Paprakis
// Author3:Jacob Tsafatinos
// Author4:Lindsay Chown
//
//
// Honor Code: I pledge that this program represents my own
//   program code and that I have coded on my own. I received 
//   help from no one in designing and debugging my program.
//   I have also read the plagiarism section in the course info
//   sheet of CSC 207 and understand the consequences.  
//*********************************************************

/**
 * A command-line interface for interacting with a file system through the use
 * of various commands and arguments. 
 * 
 * prompt is the prompt(usually the current working directory) to appear 
 * on the command line
 * promptSuffix is the string to appear after the prompt
 * 
 * @author c3chownl, c4paprak, c3tsafat, g3dakouz
 *
 */

package driver;

import java.io.IOException;
import java.util.*;

public class JShell {

	private static String prompt = "";
	private static String promptSuffix = "/#";

	/**
	 * Gets the prompt (usually the current working directory) from the JShell
	 * command line
	 * 
	 * @return the prompt written in the JSHell
	 */
	public static String getPrompt() {
		return prompt;
	}

	/**
	 * Sets the prompt (usually the current working directory) for the JShell
	 * command line
	 * 
	 * @param newPrompt
	 *            the new prompt to be written
	 */

	public static void setPrompt(String newPrompt) {
		prompt = newPrompt;
	}

	public static void main(String[] args)  {
		Directory.root = new Directory();
		Directory.root.setWorkingDirectory();
		String commandName;
		Scanner scan = new Scanner(System.in);

		/*
		 * Obtain input and split it by spaces, puts each argument into an 
		 * array
		 */
		while (true) {
			System.out.print(prompt + promptSuffix);
			commandName = scan.nextLine();
			// If we don't put this in then Eclipse won't compile it.
			if (commandName.equals("exit")) {
				break;
			}
			Input.enteredCommand(commandName);
		}
		scan.close();
		AllCommands.exitShell();
	}

}
