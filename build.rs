extern crate bindgen;

use std::env;
use std::fs::remove_file;
use std::io;
use std::path::PathBuf;
use std::process;

fn main() -> Result<()> {
    let out_dir = PathBuf::from(env::var("OUT_DIR").expect("requires $OUT_DIR env var"));

    println!("cargo:rerun-if-changed=envoy-mobile/library/common/main_interface.h");
    println!("cargo:rerun-if-changed=envoy-mobile/library/common/types/c_types.h");
    println!("cargo:rustc-link-lib=envoy_mobile");
    println!(
        "cargo:rustc-link-search={}/bazel-bin",
        out_dir.to_str().unwrap()
    );

    build_envoy_mobile(&out_dir)?;
    build_bindings(&out_dir)?;

    Ok(())
}

fn build_envoy_mobile(out_dir: &PathBuf) -> Result<()> {
    let mut builder = process::Command::new("bazel")
        .arg("build")
        .arg(format!(
            "--symlink_prefix={}/bazel-",
            out_dir.to_str().unwrap()
        ))
        .arg("//:libenvoy_mobile.so")
        .spawn()?;

    // bazel still creates a bazel-out no matter what you tell it to do. so we just remove it after
    // the fact :)
    //
    // note that we don't care if it succeeds in removing bazel-out, because Cargo will just fail
    // if it exists
    let _ = remove_file("bazel-out");

    let exit_status = builder.wait()?;
    if !exit_status.success() {
        return Err(BuildError::BazelError(exit_status.code()));
    }

    Ok(())
}

fn build_bindings(out_dir: &PathBuf) -> Result<()> {
    let bindings = bindgen::Builder::default()
        .header("wrapper.h")
        .clang_arg("-I./envoy-mobile")
        .parse_callbacks(Box::new(bindgen::CargoCallbacks))
        .generate()
        .map_err(|_| BuildError::BindingError)?;

    bindings.write_to_file(out_dir.join("bindings.rs"))?;

    Ok(())
}

#[derive(Debug)]
enum BuildError {
    IOError(io::Error),
    BazelError(Option<i32>),
    BindingError,
}

impl From<io::Error> for BuildError {
    fn from(err: io::Error) -> Self {
        BuildError::IOError(err)
    }
}

type Result<T> = core::result::Result<T, BuildError>;
