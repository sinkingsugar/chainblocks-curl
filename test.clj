(import "build/libcbcurl.so")

(def Root (Node))

(schedule
 Root
 (Chain
  "GET-it"
  "https://github.com/sinkingsugar/chainblocks"
  (Curl.Get)
  (BytesToString!!)
  (Log)))

(run Root 0.1)
