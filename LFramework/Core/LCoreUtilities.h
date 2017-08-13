/*!	\file LCoreUtilities.h
\ingroup Core
\brief ����ʵ��ģ�顣
*/

#ifndef FrameWork__LCoreUtilities_h
#define FrameWork__LCoreUtilities_h 1

#include <LFramework/Core/LException.h>
#include <LBase/algorithm.hpp>

namespace leo {
	/*!
	\throw LoggedEvent ��Χ���ʧ�ܡ�
	*/
	//@{
	//! \brief ��鴿����ֵ��ָ�����͵ķ�Χ�ڡ�
	template<typename _tDst, typename _type>
	inline _tDst
		CheckScalar(_type val, const std::string& name = {}, RecordLevel lv = Err)
	{
		using common_t = common_type_t<_tDst, _type>;

		if (LB_UNLIKELY(common_t(val) > common_t(std::numeric_limits<_tDst>::max())))
			throw LoggedEvent(name + " value out of range.", lv);
		return _tDst(val);
	}

	//! \brief ���Ǹ�������ֵ��ָ�����͵ķ�Χ�ڡ�
	template<typename _tDst, typename _type>
	inline _tDst
		CheckNonnegativeScalar(_type val, const std::string& name = {},
			RecordLevel lv = Err)
	{
		if (val < 0)
			// XXX: Use more specified exception type.
			throw LoggedEvent("Failed getting nonnegative " + name + " value.", lv);
		return CheckScalar<_tDst>(val, name, lv);
	}

	//! \brief �����������ֵ��ָ�����͵ķ�Χ�ڡ�
	template<typename _tDst, typename _type>
	inline _tDst
		CheckPositiveScalar(_type val, const std::string& name = {},
			RecordLevel lv = Err)
	{
		if (!(0 < val))
			// XXX: Use more specified exception type.
			throw LoggedEvent("Failed getting positive " + name + " value.", lv);
		return CheckScalar<_tDst>(val, name, lv);
	}

	/*!
	\brief �ж� i �Ƿ��ڱ����� [_type(0), b] �С�
	\pre ���ԣ� <tt>_type(0) < b</tt> ��
	*/
	template<typename _type>
	inline LB_STATELESS bool
		IsInClosedInterval(_type i, _type b) lnothrow
	{
		LAssert(_type(0) < b,
			"Zero element as lower bound is not less than upper bound.");
		return !(i < _type(0) || b < i);
	}
	/*!
	\brief �ж� i �Ƿ��ڱ����� [a, b] �С�
	\pre ���ԣ� <tt>a < b</tt> ��
	*/
	template<typename _type>
	inline LB_STATELESS bool
		IsInClosedInterval(_type i, _type a, _type b) lnothrow
	{
		LAssert(a < b, "Lower bound is not less than upper bound.");
		return !(i < a || b < i);
	}

	//@}
}
#endif