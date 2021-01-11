extern crate bindgen;

use std::env;
use std::io;
use std::path::PathBuf;
use std::process;

fn main() -> Result<()> {
    println!("cargo:rerun-if-changed=envoy-mobile/library/common/main_interface.h");
    println!("cargo:rerun-if-changed=envoy-mobile/library/common/types/c_types.h");
    println!("cargo:rustc-link-lib=envoy_mobile.so");
    println!("cargo:rustc-link-search=./bazel-bin");

    let out_dir = PathBuf::from(env::var("OUT_DIR").expect("requires $OUT_DIR env var"));
    build_envoy_mobile(&out_dir)?;
    build_bindings(&out_dir)?;

    Ok(())
}

fn build_envoy_mobile(out_dir: &PathBuf) -> Result<()> {
    let mut builder = process::Command::new("bazel")
        .arg(format!(
            "--output_user_root={}",
            out_dir.join("bazel-out").to_str().unwrap()
        ))
        .arg("build")
        .arg("--experimental_convenience_symlinks=ignore")
        .arg("//:envoy_mobile.so")
        .spawn()?;

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
