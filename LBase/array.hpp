/*!	\file array.hpp
\ingroup LBase
\brief ���������
*/

#ifndef LBase_array_hpp
#define LBase_array_hpp 1

#include "LBase/utility.hpp"// for lforward, common_nonvoid_t, decay_copy,
#include <array> // for std::array;
#include <algorithm> // for std::copy_n;
#include <memory> // for leo::addressof;
#include <iterator> // for std::make_move_iterator;

namespace leo {

	/*!
	\note Ϊ���⸴���ԣ��� \c std::reference_wrapper ��ʵ������������͸����
	\sa http://open-std.org/JTC1/SC22/WG21/docs/papers/2014/n4315.html
	\since build 1.4
	*/
	//@{
	/*!
	\ingroup metafunctions
	\brief ȡָ�����ͺͲ��������ƶϵ� \c std::array ʵ����
	*/
	template<typename _type = void, typename... _tParams>
	using common_array_t = std::array<common_nonvoid_t<_type, decay_t<_tParams>...>,
		sizeof...(_tParams)>;


	//! \brief ����ָ������ת����ʼ���� \c std::array ����
	template<typename _type = void, typename... _tParams>
	lconstfn common_array_t<_type, _tParams...>
		cast_array(_tParams&&... args)
	{
		return{ { decay_copy(common_nonvoid_t<_type, _tParams...>(args))... } };
	}

	//! \brief ת��ָ�������� \c std::array ����
	template<typename _type = void, typename... _tParams>
	lconstfn common_array_t<_type, _tParams...>
		forward_as_array(_tParams&&... args)
	{
		return{ { lforward(args)... } };
	}

	//! \brief ����ָ��������ʼ���� \c std::array ����
	template<typename _type = void, typename... _tParams>
	lconstfn common_array_t<_type, _tParams...>
		make_array(_tParams&&... args)
	{
		return{ { decay_copy(args)... } };
	}
	//@}

	/*!
	\brief ת��ָ������Ϊ std::array ����
	*/
	//@{
	template<typename _type, size_t _vN, typename _tSrc>
	lconstfn std::array<_type, _vN>
		to_array(const _tSrc& src)
	{
		return std::array<_type, _vN>(src);
	}
	template<typename _type, size_t _vN>
	lconstfn std::array<_type, _vN>
		to_array(const std::array<_type, _vN>& src)
	{
		return src;
	}
	template<typename _type, size_t _vN, typename _tSrcElement>
	inline std::array<_type, _vN>
		to_array(const _tSrcElement(&src)[_vN])
	{
		std::array<_type, _vN> arr;

		std::copy_n(leo::addressof(src[0]), _vN, leo::addressof(arr[0]));
		return arr;
	}
	template<typename _type, size_t _vN, typename _tSrcElement>
	inline std::array<_type, _vN>
		to_array(_tSrcElement(&&src)[_vN])
	{
		std::array<_type, _vN> arr;

		std::copy_n(std::make_move_iterator(leo::addressof(src[0])), _vN,
			leo::addressof(arr[0]));
		return arr;
	}
	//! \since build 1.3
	template<typename _type, size_t _vN, typename _tSrcElement>
	inline std::array<_type, _vN>
		to_array(std::initializer_list<_tSrcElement> il)
	{
		std::array<_type, il.size()> arr;

		std::copy_n(std::make_move_iterator(il.begin()),
			std::min(_vN, il.size()), leo::addressof(arr[0]));
		return arr;
	}
	//@}
}


#endif