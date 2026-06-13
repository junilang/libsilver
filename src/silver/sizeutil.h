bool usize_chkdflx(usize base, usize elem, usize n, usize *result) {
	return chkdmul(elem, n, &elem) || chkdadd(base, elem, result);
}

usize usize_align(usize base, usize align) {
	return (base + (align - 1)) & (~(align - 1));
}

bool usize_chkdalign(usize base, usize align, usize *result) {
	if (chkdadd(base, align - 1, &base)) return true;
	*result = base & (~(align - 1));
	return false;
}
