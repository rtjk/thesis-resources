#!/bin/sh

sig_alg_list="ed25519 ed448 dilithium2 dilithium3 dilithium5 mldsa44 mldsa65 mldsa87 falcon512 falconpadded512 falcon1024 falconpadded1024 sphincsshake128fsimple sphincsshake128ssimple sphincsshake192fsimple sphincsshake192ssimple sphincsshake256fsimple sphincsshake256ssimple mayo1 mayo2 mayo3 mayo5 CROSSrsdp128balanced CROSSrsdp128fast CROSSrsdp128small CROSSrsdp192balanced CROSSrsdp192fast CROSSrsdp192small CROSSrsdp256balanced CROSSrsdp256small CROSSrsdpg128balanced CROSSrsdpg128fast CROSSrsdpg128small CROSSrsdpg192balanced CROSSrsdpg192fast CROSSrsdpg192small CROSSrsdpg256balanced CROSSrsdpg256fast CROSSrsdpg256small"

# parameters passed on to the Dockerfile
ca_alg="ed25519"
kem="kyber512"
test_time="5"
tc_status="OFF"
tc_delay="2ms"
tc_rate="500mbps"

set -e

LANG=C date
echo CA: $ca_alg
echo KEM: $kem
echo Time: $test_time
echo Traffic Control: "$tc_status ($tc_delay $tc_rate)"
echo "--------"

for i in $sig_alg_list
	do
		echo Server: $i
		./docker-nuke.sh > /dev/null 2>&1 || true
		docker build --quiet -t oqs-curl --build-arg SIG_ALG_CA=$ca_alg --build-arg SIG_ALG_SERVER=$i --build-arg TEST_TIME=$test_time --build-arg KEM=$kem --build-arg TC_STATUS=$tc_status --build-arg TC_DELAY=$tc_delay --build-arg TC_RATE=$tc_rate . > /dev/null 2>&1
		docker run -it --cap-add=NET_ADMIN oqs-curl | grep -E "bytes|server.crt"
	done

echo "--------"
LANG=C date
