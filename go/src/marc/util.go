package marc

import "fmt"

func atoi(bytes []byte) (r int, e error) {
	r = 0
	for _, b := range bytes {
		r *= 10
		switch b {
		case '0':
			r += 0
		case '1':
			r++ // go linter bitches about r += 1 :-/
		case '2':
			r += 2
		case '3':
			r += 3
		case '4':
			r += 4
		case '5':
			r += 5
		case '6':
			r += 6
		case '7':
			r += 7
		case '8':
			r += 8
		case '9':
			r += 9
		default:
			e = fmt.Errorf("unexpected non-digit character '%c'", b)
		}
	}
	return r, e
}
