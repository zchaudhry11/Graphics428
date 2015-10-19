using UnityEngine;
using System.Collections;

public class TPSController : MonoBehaviour
{
    //Base movement
    public float speed = 5.0f;
    public float gravity = 9.0f;
    public float maxVelocityChange = 10.0f;
    public float speedDampTime = 0.1f;
    public float rotationSpeed = 8.0f;
    public float currSpeed = 0;
    //jumping
    public bool canJump = true;
    public float jumpHeight = 2.0f;

    //Jumping state
    public float jumpCooldown = 0.1f;

    //Player states
    public bool isGrounded = false;
    public bool isJumping = false;
    public bool isMoving = false;
    public bool isSprinting = false;

    //Player flags
    public bool canMove = true;

    //Player animation
    private Animator playerAnim;
    private float runLeft = 0;
    private float runRight = 0;
    private float crouchBlend = 0;
    private bool walkingBackwards = false;

    //Player Animation States
    private AnimatorStateInfo currentBaseState; //Base Layer

    static int locoState = Animator.StringToHash("Base Layer.Locomotion");

    void Awake()
    {
        playerAnim = this.GetComponent<Animator>();
        currentBaseState = playerAnim.GetCurrentAnimatorStateInfo(0);

        //Movement
        GetComponent<Rigidbody>().freezeRotation = true;
        GetComponent<Rigidbody>().useGravity = false;
    }

    void FixedUpdate()
    {
        if (jumpCooldown > 0)
        {
            jumpCooldown -= Time.deltaTime;
        }
        else if (jumpCooldown <= 0)
        {
            jumpCooldown = 0;
            playerAnim.SetBool("Jumped", false);
            canJump = true;
        }

        //Apply gravity in the y direction
        GetComponent<Rigidbody>().AddForce(0, (-gravity * GetComponent<Rigidbody>().mass), 0);
    }

    void Update()
    {
        PlayerMovement();
        PlayerCrouch();
        PlayerSprint();
        if (isGrounded)
        {
            PlayerJump();
        }

        if (walkingBackwards == false)
        {
            playerAnim.SetFloat("Velocity", currSpeed);
            playerAnim.SetBool("Backwards", false);
        }
        else
        {
            playerAnim.SetBool("Backwards", true);
        }
        
        Vector3 input = new Vector3(Input.GetAxis("Horizontal"), 0, Input.GetAxis("Vertical"));

        if (input.z < 0)
        {
            walkingBackwards = true;
        }
        else
        {
            walkingBackwards = false;
        }

        if (currSpeed >= 8) //If player caps out speed, check if they are turning for animation
        {
            if (input.x != 0 && input.z > 0) //Running forward
            {
                if (input.x > 0) //Running right
                {
                    if (runRight <= 0)
                    {
                        runRight = 0.75f;
                    }
                    if (runRight < 1)
                    {
                        runRight += 0.01f;
                    }
                    
                    playerAnim.SetFloat("Direction", runRight);
                }
                else //Running left
                {
                    if (runLeft < 0.5f)
                    {
                        runLeft += 0.01f;
                    }

                    playerAnim.SetFloat("Direction", runLeft);
                }
            }
        }

        if (input.x == 0)
        {
            runRight = 0;
            runLeft = 0;
            playerAnim.SetFloat("Direction", 0);
        }

    }

    void OnCollisionEnter(Collision col)
    {
        if (col.gameObject.tag == "Ground" || col.gameObject.tag == "Obstacle")
        {
            isGrounded = true;
            isJumping = false;
        }
    }

    void OnCollisionExit(Collision col)
    {
        if (col.gameObject.tag == "Ground" || col.gameObject.tag == "Obstacle")
        {
            isGrounded = false;
            canJump = false;
        }
    }

    void PlayerMovement()
    {
        //Get input direction and set to world space
        Vector3 targetVelocity = new Vector3(Input.GetAxis("Horizontal"), 0, Input.GetAxis("Vertical"));
        Vector3 input = new Vector3(Input.GetAxis("Horizontal"), 0, Input.GetAxis("Vertical"));
        targetVelocity = transform.TransformDirection(targetVelocity);
        targetVelocity *= currSpeed;

        //Apply force to reach target velocity
        Vector3 velocity = GetComponent<Rigidbody>().velocity;
        Vector3 velocityChange = (targetVelocity - velocity);

        //Clamp x and z speeds between the min and max range
        velocityChange.x = Mathf.Clamp(velocityChange.x, -maxVelocityChange, maxVelocityChange);
        velocityChange.z = Mathf.Clamp(velocityChange.z, -maxVelocityChange, maxVelocityChange);
        velocityChange.y = 0;

        Vector3 camFwd = new Vector3(Camera.main.transform.forward.normalized.x, 0, Camera.main.transform.forward.normalized.z); //Camera forward

        if (targetVelocity != Vector3.zero) //Rotate player
        {
            transform.rotation = Quaternion.Lerp(transform.rotation, Quaternion.LookRotation(camFwd, Vector3.up), Time.deltaTime * rotationSpeed); //Rotate player to camera forward
            this.GetComponent<Rigidbody>().AddForce(velocityChange);

            if (currSpeed < 8)
            {
                currSpeed += 0.05f;
                if (isSprinting)
                {
                    currSpeed += 0.05f; //If sprint button is held down, move player faster
                }
            }
        }
        else
        {
            if (currSpeed - speedDampTime >= 0.1f)
            {
                currSpeed -= speedDampTime;
            } else
            {
                currSpeed = 0.1f;
            }
            
        }
    }

    void PlayerJump()
    {
        //Player Jumping
        if (canJump == true && Input.GetButtonDown("Jump"))
        {
            if (jumpCooldown <= 0)
            {
                GetComponent<Rigidbody>().velocity = new Vector3(GetComponent<Rigidbody>().velocity.x, Mathf.Sqrt(2 * jumpHeight * gravity), GetComponent<Rigidbody>().velocity.z);
                isJumping = true;
                playerAnim.SetBool("Jumped", true);
                jumpCooldown = 0.1f;
            }
        }
    }

    void PlayerCrouch()
    {
        if (isGrounded && Input.GetButton("Crouch"))
        {
            crouchBlend += 0.05f;
            playerAnim.SetBool("Crouched", true);
            playerAnim.SetFloat("CrouchBlend", crouchBlend);
        }
        else
        {
            crouchBlend = 0;
            playerAnim.SetBool("Crouched", false);
        }
    }

    void PlayerSprint()
    {
        if (isGrounded && Input.GetButton("Sprint"))
        {
            isSprinting = true;
            currentBaseState = playerAnim.GetCurrentAnimatorStateInfo(0);
            if (currentBaseState.fullPathHash == locoState)
            {
                playerAnim.speed = 1.75f;
            }
        }
        else
        {
            isSprinting = false;
            playerAnim.speed = 1.0f;
        }
    }

}