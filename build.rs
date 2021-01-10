extern crate bindgen;

use std::env;
use std::io;
use std::path::PathBuf;
use std::process;

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

fn main() -> Result<()> {
    let required_files = vec![
        "envoy-mobile/library/common/main_interface.h",
        "envoy-mobile/library/common/types/c_types.h",
    ];
    for required_file in required_files.into_iter() {
        println!("cargo:rerun-if-changed={}", required_file);
    }

    println!("cargo:rustc-link-lib=envoy-mobile.so");
    println!("cargo:rustc-link-search=./bazel-bin");

    build_bindings()?;
    build_envoy_mobile()?;

    Ok(())
}

fn build_envoy_mobile() -> Result<()> {
    let mut builder = process::Command::new("bazel")
        .stdout(process::Stdio::inherit())
        .arg("build")
        .arg("//:envoy_mobile.so")
        .spawn()?;

    let exit_status = builder.wait()?;
    if !exit_status.success() {
        return Err(BuildError::BazelError(exit_status.code()));
    }

    Ok(())
}

fn build_bindings() -> Result<()> {
    let bindings = bindgen::Builder::default()
        .header("wrapper.h")
        .clang_arg("-I./envoy-mobile")
        .parse_callbacks(Box::new(bindgen::CargoCallbacks))
        .generate()
        .map_err(|_| BuildError::BindingError)?;

    let out_path = PathBuf::from(env::var("OUT_DIR").expect("requires $OUT_DIR env var"));
    bindings.write_to_file(out_path.join("bindings.rs"))?;

    Ok(())
}
