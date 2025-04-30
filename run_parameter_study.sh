#!/bin/bash

# Default values
TRACE_PREFIX="traces/app4"
DEFAULT_S=3
DEFAULT_E=4
DEFAULT_B=5
EXECUTABLE="./cache_sim"
S_VALUES=""
E_VALUES=""
B_VALUES=""

# Parse command line arguments
while [[ $# -gt 0 ]]; do
  case $1 in
    -t|--trace)
      TRACE_PREFIX="$2"
      shift 2
      ;;
    -s|--set-bits)
      DEFAULT_S="$2"
      shift 2
      ;;
    -E|--associativity)
      DEFAULT_E="$2"
      shift 2
      ;;
    -b|--block-bits)
      DEFAULT_B="$2"
      shift 2
      ;;
    -e|--executable)
      EXECUTABLE="$2"
      shift 2
      ;;
    --s-values)
      shift
      S_VALUES=""
      while [[ $# -gt 0 && ! $1 =~ ^- ]]; do
        S_VALUES+="$1 "
        shift
      done
      ;;
    --E-values)
      shift
      E_VALUES=""
      while [[ $# -gt 0 && ! $1 =~ ^- ]]; do
        E_VALUES+="$1 "
        shift
      done
      ;;
    --b-values)
      shift
      B_VALUES=""
      while [[ $# -gt 0 && ! $1 =~ ^- ]]; do
        B_VALUES+="$1 "
        shift
      done
      ;;
    -h|--help)
      echo "Usage: $0 [options]"
      echo "Options:"
      echo "  -t, --trace PREFIX      Trace file prefix (default: traces/app4)"
      echo "  -s, --set-bits S        Default set index bits (default: 3)"
      echo "  -E, --associativity E   Default associativity (default: 4)"
      echo "  -b, --block-bits B      Default block bits (default: 5)"
      echo "  -e, --executable EXE    Simulator executable (default: ./cache_sim)"
      echo "  --s-values S1 S2...     Set index bits values to test"
      echo "  --E-values E1 E2...     Associativity values to test"
      echo "  --b-values B1 B2...     Block bits values to test"
      echo "  -h, --help              Show this help message"
      exit 0
      ;;
    *)
      echo "Unknown option: $1"
      exit 1
      ;;
  esac
done

# Build the simulator
echo "Building the simulator..."
make

# Prepare arguments for the Python script
PYTHON_ARGS="-t $TRACE_PREFIX -s $DEFAULT_S -E $DEFAULT_E -b $DEFAULT_B -e $EXECUTABLE"

if [[ -n "$S_VALUES" ]]; then
  PYTHON_ARGS+=" --s-values $S_VALUES"
fi

if [[ -n "$E_VALUES" ]]; then
  PYTHON_ARGS+=" --E-values $E_VALUES"
fi

if [[ -n "$B_VALUES" ]]; then
  PYTHON_ARGS+=" --b-values $B_VALUES"
fi

# Run the parameter study
echo "Running cache parameter study with the following parameters:"
echo "  Trace prefix: $TRACE_PREFIX"
echo "  Default set index bits (s): $DEFAULT_S"
echo "  Default associativity (E): $DEFAULT_E"
echo "  Default block bits (b): $DEFAULT_B"
echo "  Executable: $EXECUTABLE"

if [[ -n "$S_VALUES" ]]; then
  echo "  Testing s values: $S_VALUES"
fi

if [[ -n "$E_VALUES" ]]; then
  echo "  Testing E values: $E_VALUES"
fi

if [[ -n "$B_VALUES" ]]; then
  echo "  Testing b values: $B_VALUES"
fi

echo "Running: python3 cache_parameter_tester.py $PYTHON_ARGS"
python3 cache_parameter_tester.py $PYTHON_ARGS

echo "Parameter study complete. Results are in the parameter_test_results directory."
echo "View the report at parameter_test_results/cache_parameter_study_report.md"
echo "Plots are available in parameter_test_results/plots/"
