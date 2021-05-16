# envoy-mobile-sys

A bindgen-generated syscrate for [envoy-mobile](https://github.com/envoyproxy/envoy-mobile).
Should not be used anywhere close to production.

## Usage

envoy-mobile uses Bazel to build. Bazel and Cargo
both like to own their builds end-to-end,
which means that they do not keep good company.
As such this library doesn't actually _build_ envoy-mobile for you,
it just provides bindings if you have envoy-mobile installed.

There is a [branch out right now](https://github.com/envoyproxy/envoy-mobile/pull/1474)
on the envoy-mobile repo to provide a `//:envoy_main_interface_lib_shared`
target, which builds a `libenvoy_mobile.so` shared object. If you're using that branch:

1. Download envoy-mobile, build `libenvoy_mobile.so`,
   and put it in a well-known location.
1. `cargo install` this syscrate into your project.
1. `cargo build`
1. ???
1. Profit!!

I recognize this build process is a bit of a headache,
especially for the Rust world where we expect a
`cargo build` to be sufficient.
I'm actively working on this problem,
so expect ot have it improve in the "near" future.
