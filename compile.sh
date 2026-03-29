gcc receiver.c common.c -o receiver -O3 -march=native -Wno-error -Wpedantic -Wall -fanalyzer -Wextra -Wunused
gcc sender.c common.c -o sender -O3 -march=native -Wno-error -Wpedantic -Wall -fanalyzer -Wextra -Wunused
