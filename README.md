# envoy-mobile-sys

A **heavily unstable** syscrate for [envoy-mobile]((https://github.com/envoyproxy/envoy-mobile).
I cannot emphasize enough that I do _not_ recommend you use this crate until it becomes more
mature. Some reasons why:

* The `envoy-mobile` submodule doesn't track upstream. It includes a much more modern version of
  envoy that hasn't been extensively tested.
* There is an additional patch in `envoy-mobile` to make it work with the fast-forwared version of
  envoy.
* I literally haven't managed to test it yet.
