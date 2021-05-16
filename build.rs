extern crate bindgen;

use std::env;
use std::io;
use std::path::PathBuf;

fn main() -> Result<()> {
    let out_dir = PathBuf::from(env::var("OUT_DIR").expect("requires $OUT_DIR env var"));

    println!("cargo:rerun-if-changed=envoy-mobile/library/common/main_interface.h");
    println!("cargo:rerun-if-changed=envoy-mobile/library/common/types/c_types.h");
    println!("cargo:rustc-link-lib=envoy_mobile");

    build_bindings(&out_dir)?;

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
    BindingError,
}

impl From<io::Error> for BuildError {
    fn from(err: io::Error) -> Self {
        BuildError::IOError(err)
    }
}

type Result<T> = core::result::Result<T, BuildError>;
