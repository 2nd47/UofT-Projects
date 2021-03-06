package driver;
import java.util.Stack;
public class StackCommands {
	
	public static Stack<Directory> newStack = new Stack<Directory>();
	
	public static Directory getTopOfStack() {
		return newStack.peek();
	}
	public static void Pushd(FileSystem dir) {
	 
	    newStack.push(Directory.workingDirectory);
	    AllCommands.changeWorkingDirectory((Directory) dir);
	}
	
	public static void Popd() {
		if (!newStack.isEmpty()) {
			AllCommands.changeWorkingDirectory(newStack.pop());
		}
		else {
			AllCommands.raiseError("Cannot pop an empty stack.");
		}
	}
	
	

}
