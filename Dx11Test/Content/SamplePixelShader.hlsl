// �s�N�Z�� �V�F�[�_�[��ʂ��ēn�����s�N�Z�����Ƃ̐F�f�[�^�B
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
};

//(��ԍς�) �F�f�[�^�̃p�X�X���[�֐��B
float4 main(PixelShaderInput input) : SV_TARGET
{
	return float4(input.color, 1.0f);
}
