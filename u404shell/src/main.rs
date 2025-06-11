use std::fs;
use std::io::{self, Write};
use std::path::{Path, PathBuf};
use std::env;

struct ShellState {
    selected_file: Option<PathBuf>,
}

impl ShellState {
    fn new() -> Self {
        Self { selected_file: None }
    }
}

fn select_file(state: &mut ShellState, file: &str) {
    state.selected_file = Some(PathBuf::from(file));
    println!("Selected {}", file);
}

fn delete_selected(state: &mut ShellState) -> io::Result<()> {
    if let Some(path) = &state.selected_file {
        if path.exists() {
            fs::remove_file(path)?;
            println!("Deleted {}", path.display());
        } else {
            println!("{} does not exist", path.display());
        }
    } else {
        println!("No file selected");
    }
    Ok(())
}

fn make_file(name: &str) -> io::Result<()> {
    fs::File::create(name)?;
    println!("Created {}", name);
    Ok(())
}

fn make_folder(name: &str) -> io::Result<()> {
    fs::create_dir_all(name)?;
    println!("Created directory {}", name);
    Ok(())
}

fn list_files() -> io::Result<()> {
    for entry in fs::read_dir(".")? {
        let entry = entry?;
        println!("{}", entry.file_name().to_string_lossy());
    }
    Ok(())
}

fn print_pwd() {
    match env::current_dir() {
        Ok(path) => println!("{}", path.display()),
        Err(e) => eprintln!("pwd: {}", e),
    }
}

fn clear_screen() {
    print!("\x1B[2J\x1B[H");
}

fn uprint(args: &[&str]) {
    println!("{}", args.join(" "));
}

fn help() {
    println!("Available commands:");
    println!("  select-file <file>");
    println!("  delete");
    println!("  makefile <file>");
    println!("  makefolder <folder>");
    println!("  ls");
    println!("  pwd");
    println!("  clear");
    println!("  uprint <message>");
    println!("  execute_script <file>");
    println!("  help");
    println!("  exit");
}

fn evaluate_condition(parts: &[&str], state: &ShellState) -> bool {
    if parts.is_empty() { return false; }
    match parts[0] {
        "file_exists" => parts.get(1).map(|p| Path::new(p).exists()).unwrap_or(false),
        "is_greater" => {
            if let (Some(a), Some(b)) = (parts.get(1), parts.get(2)) {
                let a = a.parse::<i64>().unwrap_or(0);
                let b = b.parse::<i64>().unwrap_or(0);
                a > b
            } else {
                false
            }
        }
        "is_even" => parts.get(1).and_then(|n| n.parse::<i64>().ok()).map(|n| n % 2 == 0).unwrap_or(false),
        _ => false,
    }
}

fn run_command(line: &str, state: &mut ShellState) -> io::Result<bool> {
    let parts: Vec<&str> = line.split_whitespace().collect();
    if parts.is_empty() { return Ok(true); }
    match parts[0] {
        "select-file" => {
            if let Some(f) = parts.get(1) { select_file(state, f); } else { println!("Usage: select-file <file>"); }
        }
        "delete" => { delete_selected(state)?; }
        "makefile" => {
            if let Some(f) = parts.get(1) { make_file(f)?; } else { println!("Usage: makefile <file>"); }
        }
        "makefolder" => {
            if let Some(f) = parts.get(1) { make_folder(f)?; } else { println!("Usage: makefolder <folder>"); }
        }
        "ls" => { list_files()?; }
        "pwd" => { print_pwd(); }
        "clear" => { clear_screen(); }
        "uprint" => { uprint(&parts[1..]); }
        "execute_script" => {
            if let Some(f) = parts.get(1) { run_script(Path::new(f), state)?; } else { println!("Usage: execute_script <file>"); }
        }
        "help" => { help(); }
        "exit" => { return Ok(false); }
        _ => println!("Unknown command: {}", parts[0]),
    }
    Ok(true)
}

fn run_script(path: &Path, state: &mut ShellState) -> io::Result<()> {
    let content = fs::read_to_string(path)?;
    let mut exec = true;
    let mut stack: Vec<bool> = Vec::new();
    for line in content.lines() {
        let trimmed = line.trim();
        if trimmed.is_empty() { continue; }
        if trimmed.starts_with("if ") {
            let cond_parts: Vec<&str> = trimmed[3..].split_whitespace().collect();
            let cond = evaluate_condition(&cond_parts, state);
            stack.push(exec);
            exec = exec && cond;
        } else if trimmed == "else" {
            if let Some(prev) = stack.last() {
                exec = *prev && !exec;
            }
        } else if trimmed == "endif" {
            if let Some(prev) = stack.pop() {
                exec = prev;
            }
        } else if exec {
            run_command(trimmed, state)?;
        }
    }
    Ok(())
}

fn repl() -> io::Result<()> {
    let mut state = ShellState::new();
    let stdin = io::stdin();
    loop {
        print!("> ");
        io::stdout().flush()?;
        let mut line = String::new();
        if stdin.read_line(&mut line)? == 0 { break; }
        let line = line.trim();
        if !run_command(line, &mut state)? { break; }
    }
    Ok(())
}

fn main() -> io::Result<()> {
    let args: Vec<String> = env::args().collect();
    let mut state = ShellState::new();
    if args.len() == 2 {
        run_script(Path::new(&args[1]), &mut state)
    } else {
        repl()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_is_even() {
        assert!(evaluate_condition(&["is_even", "4"], &ShellState::new()));
        assert!(!evaluate_condition(&["is_even", "5"], &ShellState::new()));
    }
}
