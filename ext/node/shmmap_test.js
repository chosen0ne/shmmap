var shmmap = require('./build/Release/shmmap');
var util = require('util');

function shmmap_log(level, msg){
	console.log('in js', level, msg);
}

shmmap.init(10000, 10000000, 'shmmap.dat', shmmap_log);

shmmap.put('abcd', '1234');

console.log(shmmap.get('abc'));

shmmap.iter(function(k, v){
	console.log(k, v);
});

console.log(util.inspect(shmmap.info()));
console.log('contains: abc', shmmap.contains('abc'));
console.log('contains: 1abc', shmmap.contains('1abc'));

console.log(shmmap.put('abc', '123456'));
