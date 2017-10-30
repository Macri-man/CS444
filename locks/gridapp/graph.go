package main

import (
	"bytes"
	"fmt"
	"io"
	"os"
	"os/exec"
	"runtime"
	"strconv"
	"strings"
	"sync"
)

var results chan result

type result struct {
	grid_size   int
	threads     int
	granularity string
	time        int
	integrity   bool
	t           string
}

func (r result) ToString() string {
	size := strconv.Itoa(r.grid_size)
	threads := strconv.Itoa(r.threads)
	time := strconv.Itoa(r.time)
	return r.t + ", " + size + ", " + threads + ", " + time
}

func run_test(size int, threads int, gran string, t string) error {
	var raw_out bytes.Buffer
	var r result
	r.grid_size = size
	r.threads = threads
	r.granularity = gran
	r.t = t

	cmd := exec.Command("./gridapp", strconv.Itoa(size), strconv.Itoa(threads), "-"+gran)
	cmd.Stdout = &raw_out
	err := cmd.Run()
	if err != nil {
		return err
	}
	for {
		line, err := raw_out.ReadString('\n')
		if err == io.EOF {
			break
		} else if err != nil {
			return err
		}
		if strings.Contains(line, "Secs elapsed:") {
			line = strings.TrimPrefix(line, "Secs elapsed:")
			line = strings.TrimSpace(line)
			r.time, _ = strconv.Atoi(line)
		} else if strings.Contains(line, "VIOLATION") {
			r.integrity = false
		} else if strings.Contains(line, "MAINTAINED") {
			r.integrity = true
		}
	}
	results <- r
	return err
}

func main() {
	runtime.GOMAXPROCS(runtime.NumCPU())
	var wg sync.WaitGroup
	var out [4]*os.File

	trials := 1
	if len(os.Args) > 1 {
		var err error
		trials, err = strconv.Atoi(os.Args[1])
		if err != nil {
			fmt.Printf("Asshole\n")
			os.Exit(-1)
		}
	}

	results = make(chan result)
	granularities := [...]string{"none", "grid", "row", "cell"}
	grid_size := 10
	for trials > 0 {
		for j, gran := range granularities {
			out[j], _ = os.OpenFile(granularities[j]+".out", os.O_RDWR|os.O_CREATE, 0666)
			defer out[j].Close()
			// test with static grid size
			for i := 1; i <= 10; i++ {
				wg.Add(1)
				go func(size int, threads int, gran string) {
					defer wg.Done()
					run_test(size, threads, gran, "grid_cons")
				}(grid_size, i, gran)
			}
			for i := 2; i <= 10; i++ {
				wg.Add(1)
				go func(size int, threads int, gran string) {
					defer wg.Done()
					run_test(size, threads, gran, "thread_cons")
				}(i, 10, gran)
			}
		}
		trials--
	}

	go func() {
		for {
			wg.Add(1)
			r := <-results
			fmt.Println(r)
			for i, v := range granularities {
				if r.granularity == v {
					io.WriteString(out[i], r.ToString()+"\n")
					break
				}
			}
			wg.Done()
		}
	}()

	wg.Wait()
}